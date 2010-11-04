// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// NPAPI extensions for NPN_* functionality.
#include "npfunctions.h"

// Main Browser function that the entire life of plugin will use.
NPNetscapeFuncs* npnfuncs = NULL;
NPNetscapeFuncs* GetNetscapeFuncs() {
  return npnfuncs;
}

// Requests creation of a new stream with the contents of the specified URL;
// gets notification of the result.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetURLNotify
NPError NP_LOADDS NPN_GetURLNotify(NPP instance, const char* url,
                                   const char* target, void* notifyData) {
  return npnfuncs->geturlnotify(instance, url, target, notifyData);
}

// Asks the browser to create a stream for the specified URL.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetURL
NPError NP_LOADDS NPN_GetURL(NPP instance,const char* url,const char* target) {
  return npnfuncs->geturl(instance, url, target);
}

// Posts data to a URL, and receives notification of the result.
// Documentation URL: https://developer.mozilla.org/en/NPN_PostURLNotify
NPError NP_LOADDS NPN_PostURLNotify(NPP instance, const char* url,
                                    const char* target, uint32_t len,
                                    const char* buf, NPBool file,
                                    void* notifyData) {
  return npnfuncs->posturlnotify(instance, url, target, len, buf,
                                   file, notifyData);
}

// Posts data to a URL.
// Documentation URL: https://developer.mozilla.org/en/NPN_PostURL
NPError NP_LOADDS NPN_PostURL(NPP instance, const char* url,const char* target,
                              uint32_t len,const char* buf, NPBool file) {
  return npnfuncs->posturl(instance, url, target, len, buf, file);
}

// 
// Documentation URL: https://developer.mozilla.org/en/NPN_PostURLNotify
NPError NP_LOADDS NPN_RequestRead(NPStream* stream, NPByteRange* rangeList) {
  return npnfuncs->requestread(stream, rangeList);
}

// Requests the creation of a new data stream produced by the plug-in and
// consumed by the browser.
// Documentation URL: https://developer.mozilla.org/en/NPN_NewStream
NPError NP_LOADDS NPN_NewStream(NPP instance, NPMIMEType type,
                                const char* target, NPStream** stream) {
  return npnfuncs->newstream(instance, type, target, stream);
}

// Pushes data into a stream produced by the plug-in and consumed by the browser.
// Documentation URL: https://developer.mozilla.org/en/NPN_Write
int32_t NP_LOADDS NPN_Write(NPP instance, NPStream* stream, int32_t len,
                            void* buffer) {
  return npnfuncs->write(instance, stream, len, buffer);
}

// Closes and deletes a stream.
// Documentation URL: https://developer.mozilla.org/en/NPN_DestroyStream
NPError NP_LOADDS NPN_DestroyStream(NPP instance, NPStream* stream,
                                    NPReason reason) {
  return npnfuncs->destroystream(instance, stream, reason);
}

// Displays a message on the status line of the browser window.
// Documentation URL: https://developer.mozilla.org/en/NPN_Status
void NP_LOADDS NPN_Status(NPP instance, const char* message) {
  npnfuncs->status(instance, message);
}

// Returns the browser's user agent field.
// Documentation URL: https://developer.mozilla.org/en/NPN_UserAgent
const char* NP_LOADDS NPN_UserAgent(NPP instance) {
  return npnfuncs->uagent(instance);
}

// Allocates memory from the browser's memory space.
// Documentation URL: https://developer.mozilla.org/en/NPN_MemAlloc
void* NP_LOADDS NPN_MemAlloc(uint32_t size) {
  return npnfuncs->memalloc(size);
}

// Allocates memory from the browser's memory space.
// Documentation URL: https://developer.mozilla.org/en/NPN_MemFree
void NP_LOADDS NPN_MemFree(void* ptr) {
  return npnfuncs->memfree(ptr);
}

// Deallocates a block of allocated memory.
// Documentation URL: https://developer.mozilla.org/en/NPN_MemAlloc
uint32_t NP_LOADDS NPN_MemFlush(uint32_t size) {
  return npnfuncs->memflush(size);
}

// Reloads all plug-ins in the Plugins directory.
// Documentation URL: https://developer.mozilla.org/en/NPN_ReloadPlugins
void NP_LOADDS NPN_ReloadPlugins(NPBool reloadPages) {
  npnfuncs->reloadplugins(reloadPages);
}

// Allows the plug-in to query the browser for information.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetValue
NPError NP_LOADDS NPN_GetValue(NPP instance, NPNVariable variable,
                               void *value) {
  return npnfuncs->getvalue(instance, variable, value);
}

// Implemented by browsers. This call is used to inform the browser of variable
// information controlled by the plugin.
// Documentation URL: https://developer.mozilla.org/en/NPN_SetValue
NPError NP_LOADDS NPN_SetValue(NPP instance,NPPVariable variable,
                               void *value) {
  return npnfuncs->setvalue(instance, variable, value);
}

// Invalidates specified drawing area prior to repainting or refreshing a
// windowless plug-in.
// Documentation URL: https://developer.mozilla.org/en/NPN_InvalidateRect
void NP_LOADDS NPN_InvalidateRect(NPP instance, NPRect *invalidRect) {
  npnfuncs->invalidaterect(instance, invalidRect);
}

// Invalidates specified drawing region prior to repainting or refreshing a
// windowless plug-in.
// Documentation URL: https://developer.mozilla.org/en/NPN_InvalidateRegion
void NP_LOADDS NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion) {
  npnfuncs->invalidateregion(instance, invalidRegion);
}

// Forces a paint message for a windowless plug-in.
// Documentation URL: https://developer.mozilla.org/en/NPN_ForceRedraw
void NP_LOADDS NPN_ForceRedraw(NPP instance) {
  npnfuncs->forceredraw(instance);
}

// Thread safe way to request that the browser calls a plugin function on the 
// browser or plugin thread (the thread on which the plugin was initiated).
// Documentation URL: https://developer.mozilla.org/en/NPN_PluginThreadAsyncCall
void NP_LOADDS NPN_PluginThreadAsyncCall(NPP instance,void (*func) (void *),
                                         void *userData) {
  npnfuncs->pluginthreadasynccall(instance, func, userData);
}

// Provides information to a plugin which is associated with a given URL, for
// example the cookies or preferred proxy.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetValueForURL
NPError NP_LOADDS NPN_GetValueForURL(NPP instance, NPNURLVariable variable,
                                     const char *url, char **value,
                                     uint32_t *len) {
  return npnfuncs->getvalueforurl(instance, variable, url, value, len);
}

// Allows a plugin to change the stored information associated with a URL, in
// particular its cookies. (While the API theoretically allows the preferred
// proxy for a given URL to be changed, doing so does not have much meaning
// given how proxies are configured, and is not supported.)
// Documentation URL: https://developer.mozilla.org/en/NPN_SetValueForURL
NPError NP_LOADDS NPN_SetValueForURL(NPP instance, NPNURLVariable variable,
                                     const char *url, const char *value,
                                     uint32_t len) {
  return npnfuncs->setvalueforurl(instance, variable, url, value, len);
}

// The function is called by plugins to get HTTP authentication information from
// the browser.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetAuthenticationInfo
NPError NP_LOADDS NPN_GetAuthenticationInfo(NPP instance,
                                            const char *protocol,
                                            const char *host, int32_t port,
                                            const char *scheme,
                                            const char *realm,
                                            char **username, uint32_t *ulen,
                                            char **password,
                                            uint32_t *plen) {
  return npnfuncs->getauthenticationinfo(instance, protocol, host, port,
                                           scheme, realm, username, ulen,
                                           password, plen);
}

// Allocates a new NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_CreateObject
NPObject *NPN_CreateObject(NPP npp, NPClass *aClass) {
  return npnfuncs->createobject(npp, aClass);
}

// Increments the reference count of the given NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_RetainObject
NPObject *NPN_RetainObject(NPObject *npobj) {
  return npnfuncs->retainobject(npobj);
}

// Decrements the reference count of the given NPObject. If the reference count
// reaches 0, the NPObject is deallocated by calling its deallocate function if
// one is provided; if one is not provided, free() is used.
// Documentation URL: https://developer.mozilla.org/en/NPN_ReleaseObject
void NPN_ReleaseObject(NPObject *npobj) {
  npnfuncs->releaseobject(npobj);
}

// Invokes a method on the given NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_Invoke
bool NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
                const NPVariant *args, uint32_t argCount, NPVariant *result) {
  return npnfuncs->invoke(npp, npobj, methodName, args, argCount, result);
}

// Invokes the default method, if one exists, on the given NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_InvokeDefault
bool NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
                       uint32_t argCount, NPVariant *result) {
  return npnfuncs->invokeDefault(npp, npobj, args, argCount, result);
}

// Evaluates a script in the scope of the specified NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_Evaluate
bool NPN_Evaluate(NPP npp, NPObject *npobj, NPString *script,
                  NPVariant *result) {
  return npnfuncs->evaluate(npp, npobj, script, result);
}

// Gets the value of a property on the specified NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetProperty
bool NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     NPVariant *result) {
  return npnfuncs->getproperty(npp, npobj, propertyName, result);
}

// Sets the value of a property on the specified NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_SetProperty
bool NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     const NPVariant *value) {
  return npnfuncs->setproperty(npp, npobj, propertyName, value);
}

// Removes a property from the specified NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_RemoveProperty
bool NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  return npnfuncs->removeproperty(npp, npobj, propertyName);
}

// Determines whether or not the specified NPObject has a particular property.
// Documentation URL: https://developer.mozilla.org/en/NPN_HasProperty
bool NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  return npnfuncs->hasproperty(npp, npobj, propertyName);
}

// Determines whether or not the specified NPObject has a particular method.
// Documentation URL: https://developer.mozilla.org/en/NPN_HasMethod
bool NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName) {
  return npnfuncs->hasmethod(npp, npobj, methodName);
}

// Gets the names of the properties and methods of the specified NPObject.
// Documentation URL: https://developer.mozilla.org/en/NPN_Enumerate
bool NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                   uint32_t *count) {
  return npnfuncs->enumerate(npp, npobj, identifier, count);
}

// Unknown.
// Documentation URL: https://developer.mozilla.org/en/NPN_Construct
bool NPN_Construct(NPP npp, NPObject *npobj, const NPVariant *args,
                   uint32_t argCount, NPVariant *result) {
  return npnfuncs->construct(npp, npobj, args, argCount, result);
}

// A plugin can call this function to indicate that a call to one of the
// plugin's NPObjects generated an error.
// Documentation URL: https://developer.mozilla.org/en/NPN_SetException
void NPN_SetException(NPObject *npobj, const NPUTF8 *message) {
  npnfuncs->setexception(npobj, message);
}

// Returns an opaque identifier for the string that is passed in.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetStringIdentifier
NPIdentifier NPN_GetStringIdentifier(const NPUTF8 *name) {
  return npnfuncs->getstringidentifier(name);
}

// Returns an array of opaque identifiers for the names that are passed in.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetStringIdentifiers
void NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount,
                              NPIdentifier *identifiers) {
  npnfuncs->getstringidentifiers(names, nameCount, identifiers);
}

// Returns an opaque identifier for the integer that is passed in.
// Documentation URL: https://developer.mozilla.org/en/NPN_GetIntIdentifier
NPIdentifier NPN_GetIntIdentifier(int32_t intid) {
  return npnfuncs->getintidentifier(intid);
}

// 
// Documentation URL: https://developer.mozilla.org/en/NPN_IdentifierIsString
bool NPN_IdentifierIsString(NPIdentifier identifier) {
  return npnfuncs->identifierisstring(identifier);
}

// Determines whether or not an identifier is a string.
// Documentation URL: https://developer.mozilla.org/en/NPN_PostURLNotify
NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier identifier) {
  return npnfuncs->utf8fromidentifier(identifier);
}

// Returns the integer value corresponding to the given integer identifier.
// Documentation URL: https://developer.mozilla.org/en/NPN_IntFromIdentifier
int32_t NPN_IntFromIdentifier(NPIdentifier identifier) {
  return npnfuncs->intfromidentifier(identifier);
}

// NPN_ReleaseVariantValue() releases the value in the given variant.
// Documentation URL: https://developer.mozilla.org/en/NPN_ReleaseVariantValue
void NPN_ReleaseVariantValue(NPVariant *variant) {
  npnfuncs->releasevariantvalue(variant);
}