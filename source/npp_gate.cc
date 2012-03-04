// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <stdio.h>
#include <new>

#include "npapi.h"
#include "win_desktop_service.h"

using set_wallpaper_extension::DesktopService;

extern "C" {

// This file implements functions that the plugin is expected to implement so
// that the browser can all them.


// Called after NP_Initialize with a Plugin Instance Pointer and context
// information for the plugin instance that is being allocated.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NPP_New
NPError NPP_New(NPMIMEType mime_type,
                NPP instance,
                uint16_t mode,
                int16_t argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved) {    
  if (instance == NULL) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  DesktopService* desktop_service = new(std::nothrow)set_wallpaper_extension::WindowsDesktopService(instance);
  if (desktop_service == NULL) {
    return NPERR_OUT_OF_MEMORY_ERROR;
  }

  instance->pdata = desktop_service;
  return NPERR_NO_ERROR;
}

// Called when a Plugin |instance| is being deleted by the browser.  This
// function should clean up any information allocated by NPP_New but not
// NP_Initialize.  Use |save| to store any information that should persist but
// note that browser may choose to throw it away.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NPP_Destroy
NPError NPP_Destroy(NPP instance, NPSavedData** save) {
  if (instance == NULL) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  DesktopService* desktop_service = static_cast<DesktopService*>(instance->pdata);
  if (desktop_service != NULL) {
    delete desktop_service;
  }
  return NPERR_NO_ERROR;
}

// NPP_GetScriptableInstance retruns the NPObject pointer that corresponds to
// NPPVpluginScriptableNPObject queried by NPP_GetValue() from the browser.
// Helper function for NPP_GetValue to create this plugin's NPObject.
// |instance| is this plugin's representation in the browser.  Returns the new
// NPObject or NULL.
// Declaration: local
// Documentation URL: N/A (not actually an NPAPI function)
NPObject* NPP_GetScriptableInstance(NPP instance) {
  if (instance == NULL) {
    return NULL;
  }

  NPObject* object = NULL;
  DesktopService* desktop_service = static_cast<DesktopService*>(instance->pdata);
  if (desktop_service) {
    object = desktop_service->GetScriptableObject();
  }
  return object;
}

// Implemented so the browser can get a scriptable instance from this plugin.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NPP_GetValue
NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
  if (NPPVpluginScriptableNPObject == variable) {
    NPObject* scriptable_object = NPP_GetScriptableInstance(instance);
    if (scriptable_object == NULL) {
      return NPERR_INVALID_INSTANCE_ERROR;
    }
    *static_cast<NPObject**>(value) = scriptable_object;
    return NPERR_NO_ERROR;
  }
  return NPERR_INVALID_PARAM;
}

// |event| just took place in this plugin's window in the browser.  This
// function should return true if the event was handled, false if it was
// ignored.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NPP_HandleEvent
int16_t NPP_HandleEvent(NPP instance, void* event) {
  return 0;
}

// Called by the browser when a new stream is started. That is, when
// NPN_GetURL(Notify) is called.
NPError NPP_NewStream(NPP instance,
                      NPMIMEType type,
                      NPStream* stream,
                      NPBool seekable,
                      uint16_t* stype) {
  // Set stype to NP_ASFILEONLY since we need the image as a file anyway to be
  // able to set the wallpaper. This causes NPP_StreamAsFile() to be called when
  // the download is complete.
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

// Called by the browser when GetURL request is complete and present as a file
// in the local filesystem.
void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
  DesktopService* desktop_service = static_cast<DesktopService*>(instance->pdata);
  desktop_service->ImageDownloadComplete(stream, fname);
}

// Called by the browser when the stream is finished.
NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
  return NPERR_NO_ERROR;
}

// If NPN_GetURLNotify is called, this function is called to indicate successful
// or unsuccessful completion of the 'get'.
void NPP_URLNotify(NPP instance,
                   const char* url,
                   NPReason reason,
                   void* notifyData) {
  DesktopService* desktop_service = static_cast<DesktopService*>(instance->pdata);
  desktop_service->DownloadCompletionStatus(url, reason);
}

} // extern "C"
