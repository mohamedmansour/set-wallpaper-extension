// Copyright 2012 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef DESKTOP_SERVICE_H_
#define DESKTOP_SERVICE_H_

#include "npapi.h"
#include "npruntime.h"

namespace set_wallpaper_extension {

class DesktopService {
 public:
  DesktopService(NPP npp);

  virtual ~DesktopService();

  // Return the primary color of the desktop as a 6-character hex color code.
  virtual bool GetSystemColor(NPVariant* result) = 0;

  // Return the current desktop wallpaper style.
  virtual bool GetWallpaperStyle(NPVariant* result) = 0;

  // Start the process of downloading 'url' to be used as a desktop background.
  virtual bool SetWallpaper(NPVariant* result, const NPString& url, int style) = 0;

  // After requesting an image with StartImageDownload(), this function will
  // be called when the image is successfully downloaded and stored on disk
  // under the name 'filename'. Implement this function to actually set the
  // desktop background.
  virtual void ImageDownloadComplete(NPStream* stream, const char* filename) = 0;

  // This function is called to indicate the success or failure of downloading
  // an image.
  virtual void DownloadCompletionStatus(const char* url, NPReason reason) = 0;

  // Although the scripting bridge is the connection between javascript world
  // and a DesktopService instance, it's convenient for a DesktopService
  // instance to own the scripting bridge because the DesktopService gets
  // created first and both should be destroyed at the same time.
  NPObject* GetScriptableObject();

  // Write a message to the console for the background.html page within Chrome.
  // Messages are written only if is_debug_ is true.
  void WriteToConsole(const char* message);

  bool is_debug() const { return is_debug_; }
  void set_is_debug(bool val) { is_debug_ = val; }

 protected:
  bool StartImageDownload(NPString image_url) const;
  NPP npp() const { return npp_; }

 private:
  NPP npp_;
  NPObject* scripting_bridge_;
  bool is_debug_;
};

} // namespace set_wallpaper_extension

#endif // DESKTOP_SERVICE_H_
