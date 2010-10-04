// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef DESKTOP_SERVICE_H_
#define DESKTOP_SERVICE_H_

#include <iostream>
#include <npapi.h>
#include <npruntime.h>
#include <npfunctions.h>
#include <string>

namespace desktop_service {

class DesktopService {
 public:
  explicit DesktopService(NPP npp, NPNetscapeFuncs* npfuncs);
  ~DesktopService();

  NPObject* GetScriptableObject();

  bool GetSystemColor(NPVariant* result);
  bool GetTileStyle(NPVariant* result);
  bool SetWallpaper(NPVariant* result, NPString path, int32_t style, int32_t tile);
  void Debug(char* message);

 private:
  NPP npp_;
  NPObject* scriptable_object_;
  NPNetscapeFuncs* npfuncs_;

  // Sets the registry for the tiles and wallpaper styles.
  // http://technet.microsoft.com/en-us/library/cc978626.aspx
  // WallpaperStyle:
  //   0 - Center the bitmap on the desktop.
  //   2 - Stretch the bitmap vertically and horizontally to fit the desktop.
  // 
  // http://technet.microsoft.com/en-us/library/cc978623.aspx
  // TileWallpaper:
  //   0 - Wallpaper is centered on the screen.
  //   1 - Wallpaper is tiled across the screen.
  void SetRegistry(int tileInt, int styleInt);

  // Get the requested image encoder class ID used for encoding from the given
  // |format| the result will be set to |pClsid|.
  int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

  // Convert the image located in |path| to JPEG and once its completed
  // successfuly, |path| will be pointed to the new image location.
  bool ConvertToJPEG(std::string* path);
};

}  // namespace desktop_service

#endif  // DESKTOP_SERVICE_H_
