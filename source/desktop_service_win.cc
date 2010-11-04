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

DesktopService::DesktopService(NPP npp)
    : npp_(npp),
      scriptable_object_(NULL),
      debug_(false) {
  ScriptingBridge::InitializeIdentifiers();
}

DesktopService::~DesktopService() {
  if (scriptable_object_)
    NPN_ReleaseObject(scriptable_object_);
}

NPObject* DesktopService::GetScriptableObject() {
  if (scriptable_object_ == NULL)
    scriptable_object_ = NPN_CreateObject(npp_, &ScriptingBridge::np_class);

  if (scriptable_object_)
    NPN_RetainObject(scriptable_object_);

  return scriptable_object_;
}

bool DesktopService::GetSystemColor(NPVariant* result) {
  char* hex_color = (char*) NPN_MemAlloc(7);
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
      (wchar_t*) NPN_MemAlloc(MAX_PATH * sizeof(wchar_t));
  HRESULT hr = URLDownloadToCacheFile(NULL, image_url.c_str(),
                                      cache_temp_path, MAX_PATH,
                                      0, NULL);

  // Check if download was successful.
  if (FAILED(hr)) {
    char error_message[35];
    sprintf(error_message, "SetWallpaper::Download::Error %04d", 
            GetLastError());
    NPN_SetException(scriptable_object_, error_message);
    SendConsole(error_message);
    NPN_MemFree(cache_temp_path);
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
    NPN_SetException(scriptable_object_, error_message);
  }

  // Cleanup.
  CoUninitialize();;
  NPN_MemFree(cache_temp_path);
  SendConsole("SetWallpaper::DONE succeeded!");
  return SUCCEEDED(hr);
}

void DesktopService::SendConsole(const char* message) {
  if (!debug_)
    return;

  // Get window object.
  NPObject* window = NULL;
  NPN_GetValue(npp_, NPNVWindowNPObject, &window);

  // Get console object.
  NPVariant consoleVar;
  NPIdentifier id = NPN_GetStringIdentifier("console");
  NPN_GetProperty(npp_, window, id, &consoleVar);
  NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

  // Get the debug object.
  id = NPN_GetStringIdentifier("debug");

  // Invoke the call with the message!
  NPVariant type;
  STRINGZ_TO_NPVARIANT(message, type);
  NPVariant args[] = { type };
  NPVariant voidResponse;
  NPN_Invoke(npp_, console, id, args,
             sizeof(args) / sizeof(args[0]),
             &voidResponse);

  // Cleanup all allocated objects, otherwise, reference count and
  // memory leaks will happen.
  NPN_ReleaseObject(window);
  NPN_ReleaseVariantValue(&consoleVar);
  NPN_ReleaseVariantValue(&voidResponse);
}

}  // namespace desktop_service
