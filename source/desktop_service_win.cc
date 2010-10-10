// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "desktop_service.h"

#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <urlmon.h>

#include "scripting_bridge.h"
#include "string_utils.h"

using desktop_service::ScriptingBridge;

namespace desktop_service {

DesktopService::DesktopService(NPP npp, NPNetscapeFuncs* npfuncs)
    : npp_(npp),
      scriptable_object_(NULL),
      npfuncs_(npfuncs),
      debug_(false) {
  ScriptingBridge::InitializeIdentifiers(npfuncs);
}

DesktopService::~DesktopService() {
  if (scriptable_object_)
    npfuncs_->releaseobject(scriptable_object_);
}

NPObject* DesktopService::GetScriptableObject() {
  if (scriptable_object_ == NULL) {
    scriptable_object_ =
        npfuncs_->createobject(npp_, &ScriptingBridge::np_class);
  }
  if (scriptable_object_)
    npfuncs_->retainobject(scriptable_object_);
  return scriptable_object_;
}

bool DesktopService::GetSystemColor(NPVariant* result) {
  char* hex_color = (char*) npfuncs_->memalloc(7);
  DWORD color = GetSysColor(1);
  sprintf(hex_color, "%02X%02X%02X", GetRValue(color), GetGValue(color),
          GetBValue(color));
  result->type = NPVariantType_String;
  result->value.stringValue.UTF8Characters = hex_color;
  result->value.stringValue.UTF8Length = 6;
  SendConsole(std::string("GetSystemColor::DONE ").append(hex_color).c_str());
  return true;
}

bool DesktopService::GetWallpaperStyle(NPVariant* result) {
  SendConsole("GetWallpaperStyle::BEGIN");
  return true;
}

bool DesktopService::SetWallpaper(NPVariant* result,
                                  NPString image_url_char,
                                  int style) {
  SendConsole("SetWallpaper::BEGIN starting");
  SendConsole(std::string("SetWallpaper::URL ").append(
      image_url_char.UTF8Characters).c_str());
  std::wstring image_url(string_utils::SysUTF8ToWide(
      image_url_char.UTF8Characters));

  // Download the image to the cache so we can set it as a wallpaper.
  // TODO(mohamed): Figure out a way to extract image from Chrome's cache.
  wchar_t* cache_temp_path =
      (wchar_t*) npfuncs_->memalloc(MAX_PATH * sizeof(wchar_t));
  HRESULT hr = URLDownloadToCacheFile(NULL, image_url.c_str(),
                                      cache_temp_path, MAX_PATH,
                                      0, NULL);

  // Check if download was successful.
  if (FAILED(hr)) {
    char error_message[35];
    sprintf(error_message, "SetWallpaper::Download::Error %04d", 
            GetLastError());
    npfuncs_->setexception(scriptable_object_, error_message);
    SendConsole(error_message);
    npfuncs_->memfree(cache_temp_path);
    return false;
  }

  SendConsole("SetWallpaper::Download success.");
  SendConsole(std::string("SetWallpaper::Path ").append(
      string_utils::SysWideToUTF8(cache_temp_path)).c_str());

  // Set the active wallpaper on the desktop.
  LPACTIVEDESKTOP active_desktop;
  WALLPAPEROPT wallpaper_options;
  CoInitialize(NULL);
  hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                        IID_IActiveDesktop, (void**)&active_desktop);
  if (SUCCEEDED(hr)) {
    hr = active_desktop->SetWallpaper(cache_temp_path, 0);
    if (SUCCEEDED(hr)) {
      wallpaper_options.dwSize = sizeof(WALLPAPEROPT);
      wallpaper_options.dwStyle = style;
      hr = active_desktop->SetWallpaperOptions(&wallpaper_options, 0);
      if (SUCCEEDED(hr)) {
        hr = active_desktop->ApplyChanges(AD_APPLY_ALL);
      } else {
        SendConsole("SetWallpaper::Options failed!");
      }
    } else {
      SendConsole("SetWallpaper::Image failed!");
    }
    active_desktop->Release();
  } else {
    SendConsole("SetWallpaper::Creation failed!");
  }

  // Error code checking.
  if (FAILED(hr)) {
    char error_message[32];
    sprintf(error_message, "SetWallpaper::Apply::Error %04d", 
            GetLastError());
    SendConsole(error_message);
    npfuncs_->setexception(scriptable_object_, error_message);
  }

  // Cleanup.
  CoUninitialize();;
  npfuncs_->memfree(cache_temp_path);
  SendConsole("SetWallpaper::DONE succeeded!");
  return SUCCEEDED(hr);
}

void DesktopService::SendConsole(const char* message) {
  if (!debug_)
    return;

  // Get window object.
  NPObject* window = NULL;
  npfuncs_->getvalue(npp_, NPNVWindowNPObject, &window);

  // Get chrome object.
  NPVariant chromeVar;
  NPIdentifier id = npfuncs_->getstringidentifier("chrome");
  npfuncs_->utf8fromidentifier(id);
  npfuncs_->getproperty(npp_, window, id, &chromeVar);
  NPObject* chrome = NPVARIANT_TO_OBJECT(chromeVar);

  // Get extension object.
  NPVariant extensionVar;
  id = npfuncs_->getstringidentifier("extension");
  npfuncs_->getproperty(npp_, chrome, id, &extensionVar);
  NPObject* extension = NPVARIANT_TO_OBJECT(extensionVar);

  // Get the getBackground object.
  NPVariant backgroundVar;
  id = npfuncs_->getstringidentifier("getBackgroundPage");
  NPVariant backgroundResponse;
  npfuncs_->invoke(npp_, extension, id, NULL, 0, &backgroundResponse);
  NPObject* background = NPVARIANT_TO_OBJECT(backgroundResponse);

  // Get console object.
  NPVariant consoleVar;
  id = npfuncs_->getstringidentifier("console");
  npfuncs_->getproperty(npp_, background, id, &consoleVar);
  NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

  // Get the debug object.
  id = npfuncs_->getstringidentifier("debug");

  // Invoke the call with the message!
  NPVariant type;
  STRINGZ_TO_NPVARIANT(message, type);
  NPVariant args[] = { type };
  NPVariant voidResponse;
  npfuncs_->invoke(npp_, console, id, args,
                   sizeof(args) / sizeof(args[0]),
                   &voidResponse);

  // Cleanup all allocated objects, otherwise, reference count and
  // memory leaks will happen.
  npfuncs_->releaseobject(window);
  npfuncs_->releasevariantvalue(&chromeVar);
  npfuncs_->releasevariantvalue(&backgroundVar);
  npfuncs_->releasevariantvalue(&consoleVar);
  npfuncs_->releasevariantvalue(&voidResponse);
}

}  // namespace desktop_service
