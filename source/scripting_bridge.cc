// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "scripting_bridge.h"

#include "desktop_service.h"

namespace desktop_service {

NPIdentifier ScriptingBridge::id_system_color;
NPIdentifier ScriptingBridge::id_wallaper;
NPIdentifier ScriptingBridge::id_tile_style;
NPIdentifier ScriptingBridge::id_debug;

// Method table for use by HasMethod and Invoke.
std::map<NPIdentifier, ScriptingBridge::MethodSelector>*
    ScriptingBridge::method_table;

// Property table for use by {Has|Get}Property.
std::map<NPIdentifier, ScriptingBridge::GetPropertySelector>*
    ScriptingBridge::get_property_table;

// Property table for use by {Set}Property.
std::map<NPIdentifier, ScriptingBridge::SetPropertySelector>*
    ScriptingBridge::set_property_table;

// Creates the plugin-side instance of NPObject.
// Called by NPN_CreateObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
NPObject* Allocate(NPP npp, NPClass* npclass) {
  return new ScriptingBridge(npp);
}

ScriptingBridge::~ScriptingBridge() {
}

// Sets up method_table and property_table.
bool ScriptingBridge::InitializeIdentifiers(NPNetscapeFuncs* npfuncs) {
  id_system_color = npfuncs->getstringidentifier("systemColor");
  id_tile_style = npfuncs->getstringidentifier("tileStyle");
  id_wallaper = npfuncs->getstringidentifier("setWallpaper");
  id_debug = npfuncs->getstringidentifier("debug");

  method_table =
      new(std::nothrow) std::map<NPIdentifier, MethodSelector>;
  if (method_table == NULL)
    return false;

  method_table->insert(
      std::pair<NPIdentifier, MethodSelector>(
          id_system_color, &ScriptingBridge::GetSystemColor));
  method_table->insert(
      std::pair<NPIdentifier, MethodSelector>(
          id_tile_style, &ScriptingBridge::GetTileStyle));
  method_table->insert(
      std::pair<NPIdentifier, MethodSelector>(
          id_wallaper, &ScriptingBridge::SetWallpaper));

  get_property_table =
      new(std::nothrow) std::map<NPIdentifier, GetPropertySelector>;
  if (get_property_table == NULL) {
    return false;
  }
  set_property_table =
      new(std::nothrow) std::map<NPIdentifier, SetPropertySelector>;
  if (set_property_table == NULL) {
    return false;
  }

  get_property_table->insert(
      std::pair<NPIdentifier, GetPropertySelector>(
          id_debug, &ScriptingBridge::GetDebug));
  set_property_table->insert(
      std::pair<NPIdentifier, SetPropertySelector>(
          id_debug, &ScriptingBridge::SetDebug));

  return true;
}

bool ScriptingBridge::GetSystemColor(const NPVariant* args,
                                     uint32_t arg_count,
                                     NPVariant* result) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service)
    return desktop_service->GetSystemColor(result);
  return false;
}

bool ScriptingBridge::GetTileStyle(const NPVariant* args,
                                   uint32_t arg_count,
                                   NPVariant* result) {
  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service)
    return desktop_service->GetTileStyle(result);
  return false;
}

bool ScriptingBridge::SetWallpaper(const NPVariant* args,
                                   uint32_t arg_count,
                                   NPVariant* result) {
  // The JavaScript signature must have three arguments, otherwise just fail
  // silently.
  if (arg_count != 3)
    return false;

  const NPVariant pathArgument = args[0];
  const NPVariant styleArgument = args[1];
  const NPVariant tileArgument = args[2];

  // Verify the arguments, we don't want to deal with any casts here.
  if (pathArgument.type != NPVariantType_String || 
      styleArgument.type != NPVariantType_Int32 || 
      tileArgument.type != NPVariantType_Int32) {
    return false;
  }

  const NPString path = NPVARIANT_TO_STRING(pathArgument);
  const int32_t style = NPVARIANT_TO_INT32(styleArgument);
  const int32_t tile = NPVARIANT_TO_INT32(tileArgument);

  DesktopService* desktop_service = static_cast<DesktopService*>(npp_->pdata);
  if (desktop_service)
    return desktop_service->SetWallpaper(result, path, style, tile);
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

// =============================================================================
//
//   NPAPI Overrides
//
// =============================================================================

// Class-specific implementation of HasMethod, used by the C-style one
// below.
bool ScriptingBridge::HasMethod(NPIdentifier name) {
  std::map<NPIdentifier, MethodSelector>::iterator i;
  i = method_table->find(name);
  return i != method_table->end();
}

// Class-specific implementation of HasProperty, used by the C-style one
// below.
bool ScriptingBridge::HasProperty(NPIdentifier name) {
  std::map<NPIdentifier, GetPropertySelector>::iterator i;
  i = get_property_table->find(name);
  return i != get_property_table->end();
}

// Class-specific implementation of GetProperty, used by the C-style one
// below.
bool ScriptingBridge::GetProperty(NPIdentifier name, NPVariant *value) {
  VOID_TO_NPVARIANT(*value);
  std::map<NPIdentifier, GetPropertySelector>::iterator i;
  i = get_property_table->find(name);
  if (i != get_property_table->end()) {
    return (this->*(i->second))(value);
  }
  return false;
}

// Class-specific implementation of SetProperty, used by the C-style one
// below.
bool ScriptingBridge::SetProperty(NPIdentifier name, const NPVariant* value) {
  std::map<NPIdentifier, SetPropertySelector>::iterator i;
  i = set_property_table->find(name);
  if (i != set_property_table->end()) {
    return (this->*(i->second))(value);
  }
  return false;
}

// Class-specific implementation of RemoveProperty, used by the C-style one
// below.
bool ScriptingBridge::RemoveProperty(NPIdentifier name) {
  return false;  // Not implemented.
}

// Class-specific implementation of InvokeDefault, used by the C-style one
// below.
bool ScriptingBridge::InvokeDefault(const NPVariant* args,
                                    uint32_t arg_count,
                                    NPVariant* result) {
  return false;  // Not implemented.
}

// Class-specific implementation of Invoke, used by the C-style one
// below.
bool ScriptingBridge::Invoke(NPIdentifier name,
                             const NPVariant* args, uint32_t arg_count,
                             NPVariant* result) {
  std::map<NPIdentifier, MethodSelector>::iterator i;
  i = method_table->find(name);
  if (i != method_table->end()) {
    return (this->*(i->second))(args, arg_count, result);
  }
  return false;
}

void ScriptingBridge::Invalidate() {
  // Not implemented.
}

// =============================================================================
//
//  Bridges NPAPI to ScriptingBridge
//
// =============================================================================

// Cleans up the plugin-side instance of an NPObject.
// Called by NPN_ReleaseObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
void Deallocate(NPObject* object) {
  delete static_cast<ScriptingBridge*>(object);
}

// Called by the browser when a plugin is being destroyed to clean up any
// remaining instances of NPClass.
// Documentation URL: https://developer.mozilla.org/en/NPClass
void Invalidate(NPObject* object) {
  return static_cast<ScriptingBridge*>(object)->Invalidate();
}

// Returns |true| if |method_name| is a recognized method.
// Called by NPN_HasMethod, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool HasMethod(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->HasMethod(name);
}

// Called by the browser to invoke a function object whose name is |name|.
// Called by NPN_Invoke, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool Invoke(NPObject* object, NPIdentifier name,
            const NPVariant* args, uint32_t arg_count,
            NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->Invoke(
      name, args, arg_count, result);
}

// Called by the browser to invoke the default method on an NPObject.
// In this case the default method just returns false.
// Apparently the plugin won't load properly if we simply
// tell the browser we don't have this method.
// Called by NPN_InvokeDefault, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool InvokeDefault(NPObject* object, const NPVariant* args, uint32_t arg_count,
                   NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->InvokeDefault(
      args, arg_count, result);
}

// Returns true if |name| is actually the name of a public property on the
// plugin class being queried.
// Called by NPN_HasProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool HasProperty(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->HasProperty(name);
}

// Returns the value of the property called |name| in |result| and true.
// Returns false if |name| is not a property on this object or something else
// goes wrong.
// Called by NPN_GetProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool GetProperty(NPObject* object, NPIdentifier name, NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->GetProperty(name, result);
}

// Sets the property |name| of |object| to |value| and return true.
// Returns false if |name| is not the name of a settable property on |object|
// or if something else goes wrong.
// Called by NPN_SetProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool SetProperty(NPObject* object, NPIdentifier name, const NPVariant* value) {
  return static_cast<ScriptingBridge*>(object)->SetProperty(name, value);
}

// Removes the property |name| from |object| and returns true.
// Returns false if it can't be removed for some reason.
// Called by NPN_RemoveProperty, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
bool RemoveProperty(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->RemoveProperty(name);
}

}  // namespace desktop_service

// Represents a class's interface, so that the browser knows what functions it
// can call on this plugin object.  The browser can use HasMethod and Invoke
// to discover the plugin class's specific interface.
// Documentation URL: https://developer.mozilla.org/en/NPClass
NPClass desktop_service::ScriptingBridge::np_class = {
  NP_CLASS_STRUCT_VERSION,
  desktop_service::Allocate,
  desktop_service::Deallocate,
  desktop_service::Invalidate,
  desktop_service::HasMethod,
  desktop_service::Invoke,
  desktop_service::InvokeDefault,
  desktop_service::HasProperty,
  desktop_service::GetProperty,
  desktop_service::SetProperty,
  desktop_service::RemoveProperty
};
