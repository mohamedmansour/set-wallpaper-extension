// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "scripting_bridge.h"

#include "desktop_service.h"

namespace set_wallpaper_extension {

ScriptingBridge::ScriptingBridge(NPP npp) : npp_(npp) {
  id_system_color_ = NPN_GetStringIdentifier("systemColor");
  id_style_ = NPN_GetStringIdentifier("wallpaperStyle");
  id_wallaper_ = NPN_GetStringIdentifier("setWallpaper");
  id_debug_ = NPN_GetStringIdentifier("debug");

  method_table_.insert(MethodMap::value_type(id_system_color_, &ScriptingBridge::GetSystemColor));
  method_table_.insert(MethodMap::value_type(id_style_, &ScriptingBridge::GetWallpaperStyle));
  method_table_.insert(MethodMap::value_type(id_wallaper_, &ScriptingBridge::SetWallpaper));

  get_property_table_.insert(GetPropMap::value_type(id_debug_, &ScriptingBridge::GetDebug));
  set_property_table_.insert(SetPropMap::value_type(id_debug_, &ScriptingBridge::SetDebug));
}

ScriptingBridge::~ScriptingBridge() {
}

// This really should return a const NPClass* but the NPN_CreateObject requires
// a non-const struct.
NPClass* ScriptingBridge::GetNPClass() {
  // Represents a class's interface, so that the browser knows what functions it
  // can call on this plugin object.  The browser can use HasMethod and Invoke
  // to discover the plugin class's specific interface.
  // Documentation URL: https://developer.mozilla.org/en/NPClass
  static NPClass npclass = {
    NP_CLASS_STRUCT_VERSION,
    &Allocate,
    &Deallocate,
    &Invalidate,
    &HasMethod,
    &Invoke,
    &InvokeDefault,
    &HasProperty,
    &GetProperty,
    &SetProperty,
    &RemoveProperty
  };
  return &npclass;
}

// =============================================================================
//
// Implementations of functions required by the NPAPI for subclasses of
// NPObject.
//
// =============================================================================

// Creates the plugin-side instance of NPObject.
// Called by NPN_CreateObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
NPObject* ScriptingBridge::Allocate(NPP npp, NPClass*) {
  return new ScriptingBridge(npp);
}

// Cleans up the plugin-side instance of an NPObject.
// Called by NPN_ReleaseObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
void ScriptingBridge::Deallocate(NPObject* object) {
  delete static_cast<ScriptingBridge*>(object);
}

// Called by the browser when a plugin is being destroyed to clean up any
// remaining instances of NPClass.
// Documentation URL: https://developer.mozilla.org/en/NPClass
void ScriptingBridge::Invalidate(NPObject*) {
  // Not implemented
}

// Returns |true| if |method_name| is a recognized method.
// Called by NPN_HasMethod, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::HasMethod(NPObject* object, NPIdentifier name) {
  ScriptingBridge* bridge = static_cast<ScriptingBridge*>(object);
  return bridge->method_table_.end() != bridge->method_table_.find(name);
}

// Called by the browser to invoke a function object whose name is |name|.
// Called by NPN_Invoke, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::Invoke(NPObject* object,
                             NPIdentifier name,
                             const NPVariant* args,
                             uint32_t arg_count,
                             NPVariant* result) {
  ScriptingBridge* bridge = static_cast<ScriptingBridge*>(object);
  MethodMap::iterator I = bridge->method_table_.find(name);
  if (I == bridge->method_table_.end()) {
    return false;
  }
  return (bridge->*(I->second))(args, arg_count, result);
}

// Called by the browser to invoke the default method on an NPObject. In this
// case the default method just returns false. Apparently the plugin won't load
// properly if we simply tell the browser we don't have this method. Called by
// NPN_InvokeDefault, declared in npruntime.h Documentation URL:
// https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::InvokeDefault(NPObject*, const NPVariant*, uint32_t, NPVariant*) {
  // Not implemented
  return false;
}

// Returns true if |name| is actually the name of a public property on the
// plugin class being queried. Called by NPN_HasProperty, declared in
// npruntime.h Documentation URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::HasProperty(NPObject* object,
                                  NPIdentifier name) {
  ScriptingBridge* bridge = static_cast<ScriptingBridge*>(object);
  return bridge->get_property_table_.end() != bridge->get_property_table_.find(name);
}

// Returns the value of the property called |name| in |result| and true.
// Returns false if |name| is not a property on this object or something else
// goes wrong. Called by NPN_GetProperty, declared in npruntime.h Documentation
// URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::GetProperty(NPObject* object,
                                  NPIdentifier name, NPVariant* result) {
  ScriptingBridge* bridge = static_cast<ScriptingBridge*>(object);
  VOID_TO_NPVARIANT(*result);
  GetPropMap::iterator I = bridge->get_property_table_.find(name);
  if (I == bridge->get_property_table_.end()) {
    return false;
  }
  return (bridge->*(I->second))(result);
}

// Sets the property |name| of |object| to |value| and return true.
// Returns false if |name| is not the name of a settable property on |object|
// or if something else goes wrong.
// Called by NPN_SetProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::SetProperty(NPObject* object,
                                  NPIdentifier name, const NPVariant* value) {
  ScriptingBridge* bridge = static_cast<ScriptingBridge*>(object);
  SetPropMap::iterator I = bridge->set_property_table_.find(name);
  if (I == bridge->set_property_table_.end()) {
    return false;
  }
  return (bridge->*(I->second))(value);
}

// Removes the property |name| from |object| and returns true.
// Returns false if it can't be removed for some reason.
// Called by NPN_RemoveProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool ScriptingBridge::RemoveProperty(NPObject*, NPIdentifier) {
  // Not implemented
  return false;
}

// =============================================================================

bool ScriptingBridge::GetSystemColor(const NPVariant* args,
                                     uint32_t arg_count,
                                     NPVariant* result) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service) {
    return desktop_service->GetSystemColor(result);
  }
  return false;
}

bool ScriptingBridge::GetWallpaperStyle(const NPVariant* args,
                                        uint32_t arg_count,
                                        NPVariant* result) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service) {
    return desktop_service->GetWallpaperStyle(result);
  }
  return false;
}

bool ScriptingBridge::SetWallpaper(const NPVariant* args,
                                   uint32_t arg_count,
                                   NPVariant* result) {
  // The JavaScript signature must have three arguments, otherwise just fail
  // silently.
  if (arg_count != 2)
    return false;

  const NPVariant pathArgument = args[0];
  const NPVariant styleArgument = args[1];

  // Verify the arguments, we don't want to deal with any casts here.
  // Chrome has this weird bug http://crbug.com/68175 that doesn't like ints.
  if (pathArgument.type != NPVariantType_String || 
      (styleArgument.type != NPVariantType_Int32 &&
       styleArgument.type != NPVariantType_Double)) {
    return false;
  }

  const NPString path = NPVARIANT_TO_STRING(pathArgument);
  int32_t style = 0;
  if (styleArgument.type == NPVariantType_Int32)
    style = NPVARIANT_TO_INT32(styleArgument);
  else
    style = (int32_t) NPVARIANT_TO_DOUBLE(styleArgument);


  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service)
    return desktop_service->SetWallpaper(result, path, style);
  return false;
}

bool ScriptingBridge::GetDebug(NPVariant* value) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service) {
    BOOLEAN_TO_NPVARIANT(desktop_service->debug(), *value);
    return true;
  }
  VOID_TO_NPVARIANT(*value);
  return false;
}

bool ScriptingBridge::SetDebug(const NPVariant* value) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (!desktop_service)
    return false;

  if (value->type != NPVariantType_Bool)
    return false;

  desktop_service->set_debug(NPVARIANT_TO_BOOLEAN(*value));
  return true;
}

}  // namespace set_wallpaper_extension

