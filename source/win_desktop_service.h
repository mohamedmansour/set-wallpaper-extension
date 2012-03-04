// Copyright 2012 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef WIN_DESKTOP_SERVICE_H_
#define WIN_DESKTOP_SERVICE_H_

#include "npfunctions.h"
#include "desktop_service.h"

namespace set_wallpaper_extension {

class WindowsDesktopService : public DesktopService {
 public:
  WindowsDesktopService(NPP npp);
  ~WindowsDesktopService();

  virtual bool GetSystemColor(NPVariant* result);
  virtual bool GetWallpaperStyle(NPVariant* result);
  virtual bool SetWallpaper(NPVariant* result, const NPString& path, int style);

  virtual void ImageDownloadComplete(NPStream* stream, const char* fname);
  virtual void DownloadCompletionStatus(const char* url, NPReason reason);

 private:
  // Get the requested image encoder class ID used for encoding from the given
  // |format| the result will be set to |pClsid|.
  // http://msdn.microsoft.com/en-us/library/ms533843(VS.85).aspx
  int GetEncoderClsid(const TCHAR* format, CLSID* pClsid);

  // Depending on the operating system, the supported images differ.
  // - Windows Vista / 7 supports JPG / BMP.
  // - Others supports just BMP.
  bool IsJPEGSupported();

 private:
  ULONG_PTR gdiplus_token_;
  int style_;
};

}  // namespace set_wallpaper_extension

#endif  // WIN_DESKTOP_SERVICE_H_
