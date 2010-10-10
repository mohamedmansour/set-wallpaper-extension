// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef DESKTOP_SERVICE_H_
#define DESKTOP_SERVICE_H_
#pragma once

#include <npfunctions.h>

namespace desktop_service {

class DesktopService {
 public:
  DesktopService(NPP npp, NPNetscapeFuncs* npfuncs);
  ~DesktopService();

  NPObject* GetScriptableObject();

  bool GetSystemColor(NPVariant* result);
  bool GetWallpaperStyle(NPVariant* result);
  bool SetWallpaper(NPVariant* result, NPString path, int style);

  bool debug() const { return debug_; }
  void set_debug(bool debug) { debug_ = debug; }

  // Send debug messages to the background.html page within chrome.
  void SendConsole(const char* message);

 private:
  NPP npp_;
  NPObject* scriptable_object_;
  NPNetscapeFuncs* npfuncs_;
  bool debug_;
};

}  // namespace desktop_service

#endif  // DESKTOP_SERVICE_H_
