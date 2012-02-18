// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SCRIPTING_BRIDGE_H_
#define SCRIPTING_BRIDGE_H_

#include <map>

#include "npapi.h"
#include "npfunctions.h"

namespace set_wallpaper_extension {
  
// The class that gets exposed to the browser code.
class ScriptingBridge : public NPObject {
 public:
  typedef bool (ScriptingBridge::*MethodSelector)(const NPVariant* args,
                                                  uint32_t arg_count,
                                                  NPVariant* result);
  typedef bool (ScriptingBridge::*GetPropertySelector)(NPVariant* value);
  typedef bool (ScriptingBridge::*SetPropertySelector)(const NPVariant* result);
  typedef std::map<NPIdentifier, MethodSelector>      MethodMap;
  typedef std::map<NPIdentifier, GetPropertySelector> GetPropMap;
  typedef std::map<NPIdentifier, SetPropertySelector> SetPropMap;

  explicit ScriptingBridge(NPP npp);
  virtual ~ScriptingBridge();

  static NPClass* GetNPClass();

  // These methods represent the NPObject implementation.  The browser calls
  // these methods by calling functions in the |np_class| struct.
  static NPObject* Allocate(NPP, NPClass*);
  static void Deallocate(NPObject*);
  static void Invalidate(NPObject*);
  static bool HasMethod(NPObject*, NPIdentifier);
  static bool Invoke(NPObject*,
                     NPIdentifier name,
                     const NPVariant* args,
                     uint32_t arg_count,
                     NPVariant* result);
  static bool InvokeDefault(NPObject*,
                            const NPVariant* args,
                            uint32_t arg_count,
                            NPVariant* result);
  static bool HasProperty(NPObject*, NPIdentifier name);
  static bool GetProperty(NPObject*, NPIdentifier name, NPVariant* result);
  static bool SetProperty(NPObject*, NPIdentifier name, const NPVariant* value);
  static bool RemoveProperty(NPObject*, NPIdentifier name);

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

  NPIdentifier id_system_color_;
  NPIdentifier id_wallaper_;
  NPIdentifier id_style_;
  NPIdentifier id_debug_;

  MethodMap method_table_;
  GetPropMap get_property_table_;
  SetPropMap set_property_table_;
};

}  // namespace set_wallpaper_extension

#endif  // SCRIPTING_BRIDGE_H_
