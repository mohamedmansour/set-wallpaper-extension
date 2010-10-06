// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "desktop_service.h"

#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <urlmon.h>

#include "scripting_bridge.h"
#include "string_utils.h"

using desktop_service::ScriptingBridge;

using namespace Gdiplus;
using namespace string_utils;

namespace desktop_service {
DesktopService::DesktopService(NPP npp, NPNetscapeFuncs* npfuncs)
    : npp_(npp),
      scriptable_object_(NULL),
      npfuncs_(npfuncs),
      gdiplus_token_(NULL) {
  ScriptingBridge::InitializeIdentifiers(npfuncs);
  GdiplusStartupInput gdiplus_startup_input;
  GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input, NULL);
}

DesktopService::~DesktopService() {
  if (gdiplus_token_)
    GdiplusShutdown(gdiplus_token_);
  if (scriptable_object_)
    npfuncs_->releaseobject(scriptable_object_);
}

NPObject* DesktopService::GetScriptableObject() {
  if (scriptable_object_ == NULL) {
    scriptable_object_ =
        npfuncs_->createobject(npp_, &ScriptingBridge::np_class);
  }
  if (scriptable_object_)
    npfuncs_->retainobject(scriptable_object_);
  return scriptable_object_;
}

bool DesktopService::GetSystemColor(NPVariant* result) {
  Debug("Trying to get system color.");
  char* hex_color = (char*)npfuncs_->memalloc(7);
  DWORD color = GetSysColor(1);
  sprintf(hex_color, "%02X%02X%02X", GetRValue(color), GetGValue(color),
          GetBValue(color));
  result->type = NPVariantType_String;
  result->value.stringValue.UTF8Characters = hex_color;
  result->value.stringValue.UTF8Length = 6;
  Debug(std::string("System color found: ").append(hex_color));
  return true;
}

bool DesktopService::GetTileStyle(NPVariant* result) {
  Debug("Getting wallpaper tile style.");
  return true;
}

bool DesktopService::SetWallpaper(NPVariant* result,
                                  NPString image_url_char,
                                  int32_t style,
                                  int32_t tile) {
  Debug(std::string("Trying to set wallpaper for ").
      append(image_url_char.UTF8Characters));
  std::wstring image_url(SysUTF8ToWide(image_url_char.UTF8Characters));

  // Lets check if its a valid file extension that the Bitmap encoder can
  // understand.
  std::wstring file_extension = PathFindExtension(image_url.c_str());
  if (file_extension.compare(L".bmp") != 0 &&
      file_extension.compare(L".gif") != 0 &&
      file_extension.compare(L".exif") != 0 &&
      file_extension.compare(L".jpg") != 0 &&
      file_extension.compare(L".jpeg") != 0 &&
      file_extension.compare(L".png") != 0 &&
      file_extension.compare(L".tiff") != 0) {
    Debug("Image format not accepted.");
    npfuncs_->setexception(scriptable_object_, "Image format not accepted.");
    return false;
  }

  wchar_t* cache_temp_path = (wchar_t*) npfuncs_->memalloc(MAX_PATH);
  HRESULT hr = URLDownloadToCacheFile(NULL, image_url.c_str(),
                                      cache_temp_path, MAX_PATH,
                                      0, NULL);
  if (FAILED(hr)) {
    Debug("Image cannot download.");
    npfuncs_->setexception(scriptable_object_, "Image cannot download.");
    return false;
  }

  Debug("Image has downloaded successfully.");
  std::wstring image_cache_path(cache_temp_path);
  npfuncs_->memfree(cache_temp_path);

  // If the file type isn't jpg or bmp, we need to convert.
  if (file_extension.compare(L".jpg") != 0 &&
      file_extension.compare(L".bmp") != 0) {
    std::wstring new_image_cache_path = ConvertToJPEG(image_cache_path);
    if (new_image_cache_path.empty()) {
      npfuncs_->setexception(scriptable_object_, "Cannot convert image.");
      return false;
    }

    // Refer to the new path since that is where our encoded image might be.
    image_cache_path = new_image_cache_path;
    Debug(std::string("Image converted successfully to ").
        append(SysWideToUTF8(new_image_cache_path)));
  }
  
  SetWallpaperStyle(tile, style);

  if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,
      const_cast<wchar_t*>(image_cache_path.c_str()), 
      SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
    Debug("Wallpaper set successfully.");
    return true;
  }
  int error = GetLastError();
  char* error_str = new char[1024];
  sprintf(error_str, "Cannot set wallpaper. Code: %d Path: %s",
           error, image_cache_path.c_str());
  Debug(error_str);
  delete[] error_str; 
  error_str = NULL;
  npfuncs_->setexception(scriptable_object_, "Cannot set wallpaper.");
  return false;
}

void DesktopService::Debug(const std::string& message) {
  // Get window object.
  NPObject* window = NULL;
  npfuncs_->getvalue(npp_, NPNVWindowNPObject, &window);

  // Get chrome object.
  NPVariant chromeVar;
  NPIdentifier id = npfuncs_->getstringidentifier("chrome");
  npfuncs_->utf8fromidentifier(id);
  npfuncs_->getproperty(npp_, window, id, &chromeVar);
  NPObject* chrome = NPVARIANT_TO_OBJECT(chromeVar);

  // Get extension object.
  NPVariant extensionVar;
  id = npfuncs_->getstringidentifier("extension");
  npfuncs_->getproperty(npp_, chrome, id, &extensionVar);
  NPObject* extension = NPVARIANT_TO_OBJECT(extensionVar);

  // Get the getBackground object.
  NPVariant backgroundVar;
  id = npfuncs_->getstringidentifier("getBackgroundPage");
  NPVariant backgroundResponse;
  npfuncs_->invoke(npp_, extension, id, NULL, 0, &backgroundResponse);
  NPObject* background = NPVARIANT_TO_OBJECT(backgroundResponse);

  // Get console object.
  NPVariant consoleVar;
  id = npfuncs_->getstringidentifier("console");
  npfuncs_->getproperty(npp_, background, id, &consoleVar);
  NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

  // Get the debug object.
  NPVariant debugVar;
  id = npfuncs_->getstringidentifier("debug");

  // Invoke the call with the message!
  NPVariant type;
  STRINGZ_TO_NPVARIANT(message.c_str(), type);
  NPVariant args[] = { type };
  NPVariant voidResponse;
  npfuncs_->invoke(npp_, console, id, args,
                   sizeof(args) / sizeof(args[0]),
                   &voidResponse);

  // Cleanup all allocated objects, otherwise, reference count and
  // memory leaks will happen.
  npfuncs_->releaseobject(window);
  npfuncs_->releasevariantvalue(&chromeVar);
  npfuncs_->releasevariantvalue(&backgroundVar);
  npfuncs_->releasevariantvalue(&consoleVar);
  npfuncs_->releasevariantvalue(&debugVar);
}

void DesktopService::SetWallpaperStyle(int tile, int style) {
  
  Debug("Trying to set wallpaper style.");

  wchar_t sub_key[] = L"Control Panel\\Desktop";
  DWORD dw_disp = 0;
  HKEY key;

  // Clamp the inputs to be from 0 to 2.
  //tile = tile < 0 ? 2 : (tile > 2 ? 2 : tile);
  //style = style < 0 ? 2 : (style > 2 ? 2 : style);

  // Try to create / open a subkey under HKCU for TileWallpaper and
  // WallpaperStyle.
  DWORD rc = RegCreateKeyEx(HKEY_CURRENT_USER, sub_key, 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, &dw_disp );
  if (rc == ERROR_SUCCESS) {
    unsigned char t[2];
    unsigned char s[2];
    t[0] = tile + '0';
    s[0] = style + '0';
    t[1] = '\0';
    s[1] = '\0';
    RegSetValueEx(key, L"TileWallpaper", 0, REG_SZ, t, sizeof(t));
    RegSetValueEx(key, L"WallpaperStyle", 0, REG_SZ, s, sizeof(s));
    RegCloseKey(key);
    Debug("Wallpaper style set sucessfully.");
  }
  else {
    Debug("Failed to set the wallpaper style.");
  }
}

int DesktopService::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
  UINT  num = 0;  // Number of image encoders.
  UINT  size = 0; // Size of the image encoder array in bytes.

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure.

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;  // Failure.

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0 ) {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success.
    }    
  }

  free(pImageCodecInfo);
  return -1;  // Failure.
}

std::wstring DesktopService::ConvertToJPEG(const std::wstring& path) {
  Debug("Image is not a known type, need to convert to JPEG.");

  // Prepare the JPEG encoder.
  CLSID jpg_clsid;
  GetEncoderClsid(L"image/jpeg", &jpg_clsid);

  // Save the file to the stream.
  Bitmap* bitmap = new Bitmap(path.c_str());
  if (!bitmap) {
    Debug("Something went wrong while constructing the bitmap.");
    return std::wstring();
  }

  // Append jpg to the file for saving.
  std::wstring renamed_image_path(path);
  renamed_image_path.append(L".jpg");
  Status stat = bitmap->Save(renamed_image_path.c_str(), &jpg_clsid, NULL);
  delete bitmap;

  bool success = true;
  if (stat != Ok) {
    int error = GetLastError();
    char* error_str = new char[1024];
    sprintf(error_str, "Cannot save image. Code: %d|%d Path: %s",
      stat, error, path.c_str());
    Debug(error_str);
    delete[] error_str;
    error_str = NULL;
    success = false;
  }

  return success ? renamed_image_path : std::wstring();
}

}