// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef DESKTOP_SERVICE_H_
#define DESKTOP_SERVICE_H_
#pragma once

#include <iostream>
#include <npapi.h>
#include <npruntime.h>
#include <npfunctions.h>
#include <string>

namespace desktop_service {

class DesktopService {
 public:
  DesktopService(NPP npp, NPNetscapeFuncs* npfuncs);
  ~DesktopService();

  NPObject* GetScriptableObject();

  bool GetSystemColor(NPVariant* result);
  bool GetTileStyle(NPVariant* result);
  bool SetWallpaper(NPVariant* result, NPString path, int32_t style,
                    int32_t tile);
  void Debug(const std::string& message);

 private:
  // Sets the registry for the wallpaper styles.
  // http://technet.microsoft.com/en-us/library/cc978626.aspx
  // WallpaperStyle:
  //   0 - Center the bitmap on the desktop.
  //   2 - Stretch the bitmap vertically and horizontally to fit the desktop.
  // 
  // http://technet.microsoft.com/en-us/library/cc978623.aspx
  // TileWallpaper:
  //   0 - Wallpaper is centered on the screen.
  //   1 - Wallpaper is tiled across the screen.
  void SetWallpaperStyle(int tile, int style);

  // Get the requested image encoder class ID used for encoding from the given
  // |format| the result will be set to |pClsid|.
  // http://msdn.microsoft.com/en-us/library/ms533843(VS.85).aspx
  int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

  // Convert the image located in |path| to JPEG and once its completed
  // successfuly, |path| will be pointed to the new image location.
  std::wstring ConvertToJPEG(const std::wstring& path);

  NPP npp_;
  NPObject* scriptable_object_;
  NPNetscapeFuncs* npfuncs_;
  ULONG_PTR gdiplus_token_;
};

}  // namespace desktop_service

#endif  // DESKTOP_SERVICE_H_
