// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "npapi.h"
#include "npfunctions.h"

extern NPNetscapeFuncs* npnfuncs;

extern "C" {
// When the browser calls NP_Initialize the plugin needs to return a list
// of functions that have been implemented so that the browser can
// communicate with the plugin.  This function populates that list,
// |nppfuncs|, with pointers to the functions.
NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *plugin_functions) {
  memset(plugin_functions, 0, sizeof(*plugin_functions));
  plugin_functions->version       = NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL;
  plugin_functions->size          = sizeof(*plugin_functions);
  plugin_functions->newp          = NPP_New;
  plugin_functions->destroy       = NPP_Destroy;
  plugin_functions->event         = NPP_HandleEvent;
  plugin_functions->getvalue      = NPP_GetValue;
  return NPERR_NO_ERROR;
}

// Provides global initialization for a plug-in.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NP_Initialize
NPError OSCALL NP_Initialize(NPNetscapeFuncs *npnf) {
  if(npnf == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;

  if(HIBYTE(npnf->version) > NP_VERSION_MAJOR)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;

  if(npnf->size < sizeof(NPNetscapeFuncs)) 
    return NPERR_INVALID_FUNCTABLE_ERROR; 

  npnfuncs = npnf;
  return NPERR_NO_ERROR;
}

// Provides global deinitialization for a plug-in.
// Declaration: npapi.h
// Documentation URL: https://developer.mozilla.org/en/NP_Shutdown
NPError OSCALL NP_Shutdown() {
	return NPERR_NO_ERROR;
}
}  // extern "C"