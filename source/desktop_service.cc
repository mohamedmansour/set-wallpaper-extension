#include "desktop_service.h"
#include "scripting_bridge.h"

namespace set_wallpaper_extension {

DesktopService::DesktopService(NPP npp)
    : npp_(npp),
      scripting_bridge_(NULL),
      is_debug_(false) {
}

DesktopService::~DesktopService()
{
  if (scripting_bridge_) {
    NPN_ReleaseObject(scripting_bridge_);
  }
}

NPObject* DesktopService::GetScriptableObject()
{
  if (NULL == scripting_bridge_) {
    scripting_bridge_ = NPN_CreateObject(npp(), ScriptingBridge::GetNPClass());
    if (NULL != scripting_bridge_) {
      NPN_RetainObject(scripting_bridge_);
    }
  }

  return scripting_bridge_;
}

void DesktopService::WriteToConsole(const char* message) {
  if (!is_debug()) {
    return;
  }

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

bool DesktopService::StartImageDownload(NPString image_url) const
{
  // Ask browser to retrieve this file for us. The actual wallpaper-setting
  // process is completed by ImageDownloadComplete().
  NPError err = NPN_GetURLNotify(npp(), image_url.UTF8Characters, 0, 0);

  return err == NPERR_NO_ERROR;
}

} // namespace set_wallpaper_extension 
