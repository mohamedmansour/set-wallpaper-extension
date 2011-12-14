// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SCRIPTING_BRIDGE_H_
#define SCRIPTING_BRIDGE_H_
#pragma once

#include <map>

#include "npapi.h"
#include "npfunctions.h"

namespace desktop_service {
  
// The class that gets exposed to the browser code.
class ScriptingBridge : public NPObject {
 public:
  typedef bool (ScriptingBridge::*MethodSelector)(const NPVariant* args,
                                                  uint32_t arg_count,
                                                  NPVariant* result);
  typedef bool (ScriptingBridge::*GetPropertySelector)(NPVariant* value);
  typedef bool (ScriptingBridge::*SetPropertySelector)(const NPVariant* result);

  explicit ScriptingBridge(NPP npp): npp_(npp) {}
  virtual ~ScriptingBridge();

  // These methods represent the NPObject implementation.  The browser calls
  // these methods by calling functions in the |np_class| struct.
  virtual void Invalidate();
  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name,
                      const NPVariant* args,
                      uint32_t arg_count,
                      NPVariant* result);
  virtual bool InvokeDefault(const NPVariant* args,
                             uint32_t arg_count,
                             NPVariant* result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant* result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant* value);
  virtual bool RemoveProperty(NPIdentifier name);

  // Initializes all the bridge identifiers from JavaScript land.
  static bool InitializeIdentifiers();

  static NPClass np_class;

  // These methods are exposed via the scripting bridge to the browser.
  // Each one is mapped to a string id, which is the name of the method that
  // the broswer sees. Each of these methods wraps a method in the associated
  // DesktopService object, which is where the actual implementation lies.

  // Gets the system background color.
  bool GetSystemColor(const NPVariant* args, uint32_t arg_count,
                      NPVariant* result);
  // Gets the tile and wallpaper style.
  bool GetWallpaperStyle(const NPVariant* args, uint32_t arg_count,
                         NPVariant* result);
  // Sets the wallpaper.
  bool SetWallpaper(const NPVariant* args, uint32_t arg_count,
                    NPVariant* result);

  // Accessor/mutator for the debug property.
  bool GetDebug(NPVariant* value);
  bool SetDebug(const NPVariant* value);

 private:
  NPP npp_;

  static NPIdentifier id_system_color;
  static NPIdentifier id_wallaper;
  static NPIdentifier id_style;
  static NPIdentifier id_debug;

  static std::map<NPIdentifier, MethodSelector>* method_table;
  static std::map<NPIdentifier, GetPropertySelector>* get_property_table;
  static std::map<NPIdentifier, SetPropertySelector>* set_property_table;
};

}  // namespace desktop_service

#endif  // SCRIPTING_BRIDGE_H_