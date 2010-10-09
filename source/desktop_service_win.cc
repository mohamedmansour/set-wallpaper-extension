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
      gdiplus_token_(NULL),
      supports_jpeg_wallpaper_(IsJPEGSupported()) {
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
  char* hex_color = (char*) npfuncs_->memalloc(7);
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
      file_extension.compare(L".jpg") != 0 &&
      file_extension.compare(L".jpeg") != 0 &&
      file_extension.compare(L".png") != 0 &&
      file_extension.compare(L".tif") != 0 &&
      file_extension.compare(L".tiff") != 0) {
    Debug("Image format not accepted.");
    npfuncs_->setexception(scriptable_object_, "Image format not accepted.");
    return false;
  }

  // Download the image to the cache so we can set it as a wallpaper.
  wchar_t* cache_temp_path =
      (wchar_t*) npfuncs_->memalloc(MAX_PATH * sizeof(wchar_t));
  HRESULT hr = URLDownloadToCacheFile(NULL, image_url.c_str(),
                                      cache_temp_path, MAX_PATH,
                                      0, NULL);
  if (FAILED(hr)) {
    Debug("Image cannot be downloaded.");
    npfuncs_->setexception(scriptable_object_, "Image cannot downloaded.");
    npfuncs_->memfree(cache_temp_path);
    return false;
  }

  Debug("Image has downloaded successfully.");
  std::wstring image_cache_path(cache_temp_path);
  npfuncs_->memfree(cache_temp_path);

  // BMP is a supported wallpaper image type for all windows. So we can skip
  // the conversion process.
  if (file_extension.compare(L".bmp") != 0) {
    std::wstring new_image_cache_path;

    // Check if the operating supports JPEG, if so, we can convert to that type
    // instead of forcing all conversions to BMP. Only if needed.
    if (supports_jpeg_wallpaper_) {
      if (file_extension.compare(L".jpg") != 0 &&
          file_extension.compare(L".jpeg") != 0) {
        new_image_cache_path = ConvertToFileType(L"jpeg", image_cache_path);
      }
      else {
        new_image_cache_path = image_cache_path;
      }
    } else {
      new_image_cache_path = ConvertToFileType(L"bmp", image_cache_path);
    }

    // Something happened while converting, halt everything.
    if (new_image_cache_path.empty()) {
      npfuncs_->setexception(scriptable_object_,
                             "Cannot convert to BMP image.");
      return false;
    }

    // Refer to the new path since that is where our encoded image might be.
    image_cache_path = new_image_cache_path;
    Debug(std::string("Image converted successfully to ").
          append(SysWideToUTF8(new_image_cache_path)));
  }

  // We need to set the tile and style of the wallpaper that the user specified.
  SetWallpaperStyle(tile, style);

  // Query the Windows internals that a wallpaper needs to change.
  if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,
      const_cast<wchar_t*>(image_cache_path.c_str()), 
      SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
    Debug("Wallpaper set successfully.");
    return true;
  }

  // Error occurred setting the wallpaper.
  int error = GetLastError();
  char* error_str = new char[1024];
  sprintf(error_str, "Cannot set wallpaper. Code: %d Path: %S",
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
  wchar_t sub_key[] = L"Control Panel\\Desktop";
  DWORD dw_disp = 0;
  HKEY key;

  // Clamp the inputs to be from 0 to 2.
  tile = tile < 0 ? 0 : (tile > 2 ? 2 : tile);
  style = style < 0 ? 0 : (style > 2 ? 2 : style);

  // Try to create / open a subkey under HKCU for TileWallpaper and
  // WallpaperStyle.
  DWORD rc = RegCreateKeyEx(HKEY_CURRENT_USER, sub_key, 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, &dw_disp );
  if (rc == ERROR_SUCCESS) {
    // Since we are dealing with unicode, we need to send unicode values
    // when we store the registry.
    wchar_t tile_value[32];
    wchar_t style_value[32];
    wsprintf(tile_value, L"%u", tile);
    wsprintf(style_value, L"%u", style);
    RegSetValueEx(key, L"TileWallpaper", 0, REG_SZ, (BYTE*) tile_value,
                  (lstrlen(tile_value) + 1) * sizeof(wchar_t));
    RegSetValueEx(key, L"WallpaperStyle", 0, REG_SZ, (BYTE*) style_value,
                  (lstrlen(style_value) + 1) * sizeof(wchar_t));
    RegCloseKey(key);
    Debug("Wallpaper style set sucessfully.");
  }
  else {
    Debug("Failed to set the wallpaper style.");
  }
}

int DesktopService::GetEncoderClsid(const std::wstring& format, CLSID* pClsid) {
  UINT  num = 0;  // Number of image encoders.
  UINT  size = 0; // Size of the image encoder array in bytes.

  ImageCodecInfo* image_codec_info = NULL;

  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure.

  image_codec_info = (ImageCodecInfo*)(malloc(size));
  if (image_codec_info == NULL)
    return -1;  // Failure.

  GetImageEncoders(num, size, image_codec_info);

  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(image_codec_info[j].MimeType, format.c_str()) == 0 ) {
      *pClsid = image_codec_info[j].Clsid;
      free(image_codec_info);
      return j;  // Success.
    }    
  }

  free(image_codec_info);
  return -1;  // Failure.
}

std::wstring DesktopService::ConvertToFileType(const std::wstring& filetype,
                                               const std::wstring& path) {
  // Prepare the JPEG encoder.
  std::wstring mime_type(L"image/");
  mime_type.append(filetype);
  CLSID clsid;
  GetEncoderClsid(mime_type.c_str(), &clsid);

  // Save the file to the stream.
  Bitmap* bitmap = new Bitmap(path.c_str());
  if (!bitmap) {
    Debug("Something went wrong while constructing the image.");
    return std::wstring();
  }

  // Append jpg to the file for saving.
  std::wstring renamed_image_path(path);
  renamed_image_path.append(L".");
  renamed_image_path.append(filetype);
  Status stat = bitmap->Save(renamed_image_path.c_str(), &clsid, NULL);
  delete bitmap;

  if (stat != Ok) {
    int error = GetLastError();
    char* error_str = new char[1024];
    sprintf(error_str, "Cannot save image. Code: %d|%d Path: %S",
            stat, error, path.c_str());
    Debug(error_str);
    delete[] error_str;
    error_str = NULL;
  }

  return stat == Ok ? renamed_image_path : std::wstring();
}

bool DesktopService::IsJPEGSupported() {
  OSVERSIONINFOEX osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  int bOsVersionInfoEx;

  if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi))) {
    Debug("Cannot retrieve Windows Version.");
    return false;
  }
  if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&  osvi.dwMajorVersion > 5) {
    Debug("Windows Version supports JPEG as wallpaper.");
    return true;
  }
  Debug("Windows Version does not support JPEG as wallpaper.");
  return false;
}

}