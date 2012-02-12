// Copyright 2012 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "desktop_service.h"

#include <windows.h>
#include <gdiplus.h>
#include <wininet.h>
#include <shlobj.h>
#include <urlmon.h>
#include <userenv.h>
#include <sstream>

#include "scripting_bridge.h"

using desktop_service::ScriptingBridge;
using namespace Gdiplus;

namespace desktop_service {

DesktopService::DesktopService(NPP npp)
    : npp_(npp),
      scriptable_object_(NULL),
      gdiplus_token_(NULL),
      debug_(false),
      style_(0),
      supports_jpeg_wallpaper_(IsJPEGSupported()) {
  ScriptingBridge::InitializeIdentifiers();
  GdiplusStartupInput gdiplus_startup_input;
  GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input, NULL);
}

DesktopService::~DesktopService() {
  if (gdiplus_token_)
    GdiplusShutdown(gdiplus_token_);
  if (scriptable_object_)
    NPN_ReleaseObject(scriptable_object_);
}

NPObject* DesktopService::GetScriptableObject() {
  if (scriptable_object_ == NULL)
    scriptable_object_ = NPN_CreateObject(npp_, &ScriptingBridge::np_class);

  if (scriptable_object_)
    NPN_RetainObject(scriptable_object_);

  return scriptable_object_;
}

bool DesktopService::GetSystemColor(NPVariant* result) {
  char* hex_color = (char*) NPN_MemAlloc(7);
  DWORD color = GetSysColor(1);
  sprintf(hex_color, "%02X%02X%02X", GetRValue(color), GetGValue(color),
          GetBValue(color));
  result->type = NPVariantType_String;
  result->value.stringValue.UTF8Characters = hex_color;
  result->value.stringValue.UTF8Length = 6;
  
  std::ostringstream oss;
  oss << "GetSystemColor::DONE  " << hex_color;
  SendConsole(oss.str().c_str());

  return true;
}

bool DesktopService::GetWallpaperStyle(NPVariant* result) {
  SendConsole("GetWallpaperStyle::BEGIN");
  return true;
}

bool DesktopService::SetWallpaper(NPVariant* result,
                                  NPString image_url,
                                  int style) {
  std::ostringstream oss;
  oss << "SetWallpaper::URL " << image_url.UTF8Characters;
  SendConsole(oss.str().c_str());

  // Save the style type for later one after we recieve the image.
  style_ = style;

  // Ask browser to retrieve this file for us. The actual wallpaper-setting process
  // is completed by ImgArrived().
  NPError err = NPN_GetURLNotify(npp_, image_url.UTF8Characters, 0, 0);

  return err == NPERR_NO_ERROR;
}

void DesktopService::SendConsole(const char* message) {
  if (!debug_)
    return;

  // Get window object.
  NPObject* window = NULL;
  NPN_GetValue(npp_, NPNVWindowNPObject, &window);

  // Get console object.
  NPVariant consoleVar;
  NPIdentifier id = NPN_GetStringIdentifier("console");
  NPN_GetProperty(npp_, window, id, &consoleVar);
  NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

  // Get the debug object.
  id = NPN_GetStringIdentifier("debug");

  // Invoke the call with the message!
  NPVariant type;
  STRINGZ_TO_NPVARIANT(message, type);
  NPVariant args[] = { type };
  NPVariant voidResponse;
  NPN_Invoke(npp_, console, id, args,
             sizeof(args) / sizeof(args[0]),
             &voidResponse);

  // Cleanup all allocated objects, otherwise, reference count and
  // memory leaks will happen.
  NPN_ReleaseObject(window);
  NPN_ReleaseVariantValue(&consoleVar);
  NPN_ReleaseVariantValue(&voidResponse);
}

void DesktopService::SendError(const char* message) {
  char error_message[35];
  sprintf(error_message, "ERROR: %s %04d", message, GetLastError());
  NPN_SetException(scriptable_object_, error_message);
  SendConsole(error_message);
}

void DesktopService::NewStream(NPStream* stream) {
  std::ostringstream oss;
  oss << "New stream opened: " << stream;
  SendConsole(oss.str().c_str());
}

class COMCleanup {
public:
  ~COMCleanup()
  {
    CoUninitialize();
  }
};

void DesktopService::ImgArrived(NPStream* stream, const char* img_path) {
  // Image has arrived. Finish setting wallpaper.
  std::ostringstream oss;
  oss << "Stream " << stream << " resulted in file downloaded to " << img_path;
  SendConsole(oss.str().c_str());

  // Copy from char* to TCHAR* for SetWallpaper's benefit.
  size_t path_len = std::strlen(img_path);
  WCHAR* temp_file_path = new WCHAR[path_len + 1];
  temp_file_path[path_len] = 0;
  std::copy(img_path, img_path + path_len, temp_file_path);

  // Construct the permanent location path since we don't want to store it
  // in temporary directory. Some users remove that daily and when they restart
  // it will remove the desktop.
  WCHAR current_path[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, current_path);
  WCHAR file_name[MAX_PATH];
  wsprintf(file_name, L"%s\\SetWallpaperExtensionImage", current_path);

  // Get the char representation so we can send it to debug.
  size_t file_name_len = wcslen(file_name) + 1;
  char file_name_chars[MAX_PATH];
  size_t converted_chars = 0;
  wcstombs_s(&converted_chars, file_name_chars, file_name_len, file_name, _TRUNCATE);
  oss.str("");

  // Version 5+ of Windows has smarter image extension support. We need to
  // catch the ones that support Windows XP/2000 since they don't like JPEG,
  // they only like BMP.
  if (IsJPEGSupported()) {

    // Just copy the file to the permanent location.
    if (CopyFileW(temp_file_path, file_name, false)) {
      oss << "Copied wallpaper to " << file_name_chars;
      SendConsole(oss.str().c_str());
    }
    else {
      oss << "Something went wrong while copying image: " << file_name_chars;
      SendError(oss.str().c_str());
      return;
    }
  }
  else {
    // We need to convert it to a BMP and store it in the permanent location.
    Image* image = new Image(temp_file_path);
    if (!image) {
      SendError("Something went wrong while constructing the image.");
      return;
    }

    // Encode it to BMP.
    CLSID clsid;
    GetEncoderClsid(L"image/bmp", &clsid);
    if (image->Save(file_name, &clsid) != Ok) {
      SendError("Something went wrong while saving the image as a BMP.");
      delete image;
      return;
    }
    delete image;

    oss << "Saved wallpaper to " << file_name_chars;
    SendConsole(oss.str().c_str());
  }

  // Set the active wallpaper on the desktop.
  LPACTIVEDESKTOP active_desktop;
  WALLPAPEROPT wallpaper_options;
  CoInitialize(NULL);
  HRESULT hr;
  hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                        IID_IActiveDesktop, (void**)&active_desktop);
  if (FAILED(hr)) {
    SendError("SetWallpaper::Creation failed!");
    return;
  }

  // Perform COM cleanup when this object goes out of scope.
  COMCleanup com_cleanup;

  hr = active_desktop->SetWallpaper(file_name, 0);
  if (FAILED(hr)) {
    SendError("SetWallpaper::Image failed!");
    return;
  }

  wallpaper_options.dwSize = sizeof(WALLPAPEROPT);
  wallpaper_options.dwStyle = style_;
  hr = active_desktop->SetWallpaperOptions(&wallpaper_options, 0);
  if (FAILED(hr)) {
    SendError("SetWallpaper::Options failed!");
    return;
  }

  hr = active_desktop->ApplyChanges(AD_APPLY_ALL);
  if (FAILED(hr)) {
    SendError("SetWallpaper::Apply::Error");
    return;
  }

  SendConsole("SetWallpaper success!");
}

void DesktopService::StreamDone(NPStream* stream, NPReason reason) {
  std::ostringstream oss;
  oss << "Stream " << stream << " done. Reason: ";
  switch (reason) {
  case NPRES_DONE:
    oss << "NPRES_DONE";
    break;

  case NPRES_USER_BREAK:
    oss << "NPRES_USER_BREAK";
    break;

  case NPRES_NETWORK_ERR:
    oss << "NPRES_NETWORK_ERR";
    break;
  }
  SendConsole(oss.str().c_str());
}

void DesktopService::UrlNotify(const char* url, NPReason reason) {
  std::ostringstream oss;
  oss << "GetURL of " << url << " done. Reason: ";
  switch (reason) {
  case NPRES_DONE:
    oss << "NPRES_DONE";
    break;

  case NPRES_USER_BREAK:
    oss << "NPRES_USER_BREAK";
    break;

  case NPRES_NETWORK_ERR:
    oss << "NPRES_NETWORK_ERR";
    break;
  }
  SendConsole(oss.str().c_str());
}

int DesktopService::GetEncoderClsid(const TCHAR* format, CLSID* pClsid) {
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
    if (wcscmp(image_codec_info[j].MimeType, format) == 0 ) {
      *pClsid = image_codec_info[j].Clsid;
      free(image_codec_info);
      return j;  // Success.
    }
  }

  free(image_codec_info);
  return -1;  // Failure.
}

bool DesktopService::IsJPEGSupported() {
  OSVERSIONINFOEX osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  int bOsVersionInfoEx;

  if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi))) {
    SendConsole("Cannot retrieve Windows Version.");
    return false;
  }
  if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&  osvi.dwMajorVersion > 5) {
    SendConsole("Windows Version supports JPEG as wallpaper.");
    return true;
  }
  SendConsole("Windows Version does not support JPEG as wallpaper.");
  return false;
}

}  // namespace desktop_service
