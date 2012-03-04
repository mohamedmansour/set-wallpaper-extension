// Copyright 2012 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "win_desktop_service.h"

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <wininet.h>
#include <shlobj.h>
#include <urlmon.h>
#include <userenv.h>
#include <sstream>

#include "scripting_bridge.h"

#define CONSOLE_LOG(x) \
do { \
  std::ostringstream oss; \
  oss << x; \
  WriteToConsole(oss.str().c_str()); \
} while (0)
  
#define CONSOLE_ERR(x) \
do { \
  std::ostringstream oss; \
  oss << "ERROR: " << x << " " << GetLastError(); \
  NPN_SetException(GetScriptableObject(), oss.str().c_str()); \
  WriteToConsole(oss.str().c_str()); \
} while (0)

using namespace Gdiplus;

namespace set_wallpaper_extension {

WindowsDesktopService::WindowsDesktopService(NPP npp)
    : DesktopService(npp),
      gdiplus_token_(NULL),
      style_(0) {
  GdiplusStartupInput gdiplus_startup_input;
  GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input, NULL);
}

WindowsDesktopService::~WindowsDesktopService() {
  if (gdiplus_token_)
    GdiplusShutdown(gdiplus_token_);
}

bool WindowsDesktopService::GetSystemColor(NPVariant* result) {
  char* hex_color = (char*) NPN_MemAlloc(7);
  DWORD color = GetSysColor(1);
  sprintf(hex_color, "%02X%02X%02X", GetRValue(color), GetGValue(color),
          GetBValue(color));
  result->type = NPVariantType_String;
  result->value.stringValue.UTF8Characters = hex_color;
  result->value.stringValue.UTF8Length = 6;
  
  CONSOLE_LOG("GetSystemColor::DONE  " << hex_color);

  return true;
}

bool WindowsDesktopService::GetWallpaperStyle(NPVariant* result) {
  CONSOLE_LOG("GetWallpaperStyle::BEGIN");
  return true;
}

bool WindowsDesktopService::SetWallpaper(NPVariant* result,
                                         const NPString& image_url,
                                         int style) {
  CONSOLE_LOG("SetWallpaper::URL " << image_url.UTF8Characters);

  // Save the style type for later one after we recieve the image.
  style_ = style;

  return StartImageDownload(image_url);
}

class ScopedCOMInitialize {
public:
  ScopedCOMInitialize()
  {
    CoInitialize(NULL);
  }

  ~ScopedCOMInitialize()
  {
    CoUninitialize();
  }
};

void WindowsDesktopService::ImageDownloadComplete(NPStream* stream, const char* img_path) {
  // Image has arrived. Finish setting wallpaper.
  CONSOLE_LOG("Stream " << stream << " resulted in file downloaded to " << img_path);

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

  ScopedCOMInitialize com;

  // Use IActiveDesktop for setting wallpaper and wallpaper options. This
  // method seems to be the simplest and is supported on Win2K and later.
  HRESULT hr;
  LPACTIVEDESKTOP active_desktop;
  hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                        IID_IActiveDesktop, (void**)&active_desktop);
  if (FAILED(hr)) {
    CONSOLE_ERR("SetWallpaper::Creation failed!");
    return;
  }

  // The rules of which version of Windows supports which image formats for a
  // desktop background are seemingly non-trivial. So we opt to always convert
  // to the lowest common demoniator, BMP, and use the resulting file as the
  // desktop wallpaper.
  wsprintf(file_name, L"%s\\SetWallpaperExtensionImage.bmp", current_path);

  // Get file_name in multi-byte form for the purpose of log messages.
  char file_name_chars[MAX_PATH];
  size_t file_name_len = wcslen(file_name) + 1;
  size_t converted_chars = 0;
  wcstombs_s(&converted_chars, file_name_chars, file_name_len, file_name, _TRUNCATE);

  // Open existing file as an image.
  std::auto_ptr<Image> image(new Image(temp_file_path));
  if (NULL == image.get()) {
    CONSOLE_ERR("Something went wrong reading the downloaded image.");
    return;
  }

  // Encode it to BMP.
  CLSID clsid;
  GetEncoderClsid(L"image/bmp", &clsid);
  if (Ok != image->Save(file_name, &clsid)) {
    CONSOLE_ERR("Something went wrong while converting the image to BMP.");
    return;
  }

  CONSOLE_LOG("Converted and saved wallpaper to " << file_name_chars);

  hr = active_desktop->SetWallpaper(file_name, 0);
  if (FAILED(hr)) {
    CONSOLE_ERR("SetWallpaper::Image failed!");
    return;
  }

  WALLPAPEROPT wallpaper_options;
  wallpaper_options.dwSize = sizeof(WALLPAPEROPT);
  wallpaper_options.dwStyle = style_;
  hr = active_desktop->SetWallpaperOptions(&wallpaper_options, 0);
  if (FAILED(hr)) {
    CONSOLE_ERR("SetWallpaper::Options failed!");
    return;
  }

  hr = active_desktop->ApplyChanges(AD_APPLY_ALL);
  if (FAILED(hr)) {
    CONSOLE_ERR("SetWallpaper::Apply::Error");
    return;
  }

  CONSOLE_LOG("SetWallpaper success!");
}

void WindowsDesktopService::DownloadCompletionStatus(const char* url, NPReason reason) {
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
  WriteToConsole(oss.str().c_str());
}

int WindowsDesktopService::GetEncoderClsid(const TCHAR* format, CLSID* pClsid) {
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

bool WindowsDesktopService::IsJPEGSupported() {
  OSVERSIONINFOEX osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  int bOsVersionInfoEx;

  if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi))) {
    CONSOLE_LOG("Cannot retrieve Windows Version.");
    return false;
  }
  if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&  osvi.dwMajorVersion > 5) {
    CONSOLE_LOG("Windows Version supports JPEG as wallpaper.");
    return true;
  }
  CONSOLE_LOG("Windows Version does not support JPEG as wallpaper.");
  return false;
}

}  // namespace set_wallpaper_extension
