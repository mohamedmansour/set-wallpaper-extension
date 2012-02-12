// Copyright 2010 Mohamed Mansour. All rights reserved.
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
  SendConsole(std::string("GetSystemColor::DONE ").append(hex_color).c_str());
  return true;
}

bool DesktopService::GetWallpaperStyle(NPVariant* result) {
  SendConsole("GetWallpaperStyle::BEGIN");
  return true;
}

bool DesktopService::SetWallpaper(NPVariant* result,
                                  NPString image_url,
                                  int style) {
  SendConsole("SetWallpaper::BEGIN starting");
  SendConsole(std::string("SetWallpaper::URL ").append(image_url.UTF8Characters).c_str());

  m_style = style;

  // Ask browser to retrieve this file for us. The actual wallpaper-setting process
  // is completed by img_arrived().
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
  sprintf(error_message, "%s %04d", message, GetLastError());
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

  // Set the active wallpaper on the desktop.
  LPACTIVEDESKTOP active_desktop;
  WALLPAPEROPT wallpaper_options;
  CoInitialize(NULL);
  HRESULT hr;
  hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                        IID_IActiveDesktop, (void**)&active_desktop);
  if (FAILED(hr)) {
    SendConsole("SetWallpaper::Creation failed!");
    return;
  }

  // Perform COM cleanup when this object goes out of scope.
  COMCleanup com_cleanup;

  // Copy from char* to TCHAR* for SetWallpaper's benefit.
  size_t path_len = std::strlen(img_path);
  TCHAR* img_path_tchar = new TCHAR[path_len + 1];
  img_path_tchar[path_len] = 0;
  std::copy(img_path, img_path + path_len, img_path_tchar);

  hr = active_desktop->SetWallpaper(img_path_tchar, 0);
  if (FAILED(hr)) {
    SendConsole("SetWallpaper::Image failed!");
    return;
  }

  wallpaper_options.dwSize = sizeof(WALLPAPEROPT);
  wallpaper_options.dwStyle = m_style;
  hr = active_desktop->SetWallpaperOptions(&wallpaper_options, 0);
  if (FAILED(hr)) {
    SendConsole("SetWallpaper::Options failed!");
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
