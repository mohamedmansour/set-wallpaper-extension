// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "desktop_service.h"

#include <windows.h>
#include <gdiplus.h>
#include <prtypes.h>
#include <shlwapi.h>
#include <urlmon.h>

#include "scripting_bridge.h"

using desktop_service::ScriptingBridge;

using namespace std;
using namespace Gdiplus;

namespace desktop_service {
DesktopService::DesktopService(NPP npp, NPNetscapeFuncs* npfuncs)
    : npp_(npp),
      scriptable_object_(NULL),
      npfuncs_(npfuncs) {
  ScriptingBridge::InitializeIdentifiers(npfuncs);
}

DesktopService::~DesktopService() {
  if (scriptable_object_) {
    npfuncs_->releaseobject(scriptable_object_);
  }
}

NPObject* DesktopService::GetScriptableObject() {
  if (scriptable_object_ == NULL) {
    scriptable_object_ =
      npfuncs_->createobject(npp_, &ScriptingBridge::np_class);
  }
  if (scriptable_object_) {
    npfuncs_->retainobject(scriptable_object_);
  }
  return scriptable_object_;
}

bool DesktopService::GetSystemColor(NPVariant* result) {
  char* hexColor = (char *)npfuncs_->memalloc(7);
  DWORD color = GetSysColor(1);
  sprintf(hexColor, "%02X%02X%02X", GetRValue(color), GetGValue(color),
          GetBValue(color));
  result->type = NPVariantType_String;
  result->value.stringValue.UTF8Characters = hexColor;
  result->value.stringValue.UTF8Length = 6;
  return true;
}

bool DesktopService::GetTileStyle(NPVariant* result) {
  return true;
}

bool DesktopService::SetWallpaper(NPVariant* result,
                                  NPString path,
                                  int32_t styleInt,
                                  int32_t tileInt) {
  // Lets check if its a valid file extension that the Bitmap encoder can
  // understand.
  LPCTSTR file_extension = PathFindExtension(path.UTF8Characters);
  if (lstrcmpi(file_extension, ".bmp") != 0 &&
      lstrcmpi(file_extension, ".gif") != 0 &&
      lstrcmpi(file_extension, ".exif") != 0 &&
      lstrcmpi(file_extension, ".jpg") != 0 &&
      lstrcmpi(file_extension, ".jpeg") != 0 &&
      lstrcmpi(file_extension, ".png") != 0 &&
      lstrcmpi(file_extension, ".tiff") != 0) {
    Debug("Image format not accepted.");
    npfuncs_->setexception(scriptable_object_, "Image format not accepted.");
    return false;
  }

  const uint len = path.UTF8Length;
  char* pathChar = (char*)npfuncs_->memalloc(len + 1);
  HRESULT hr = URLDownloadToCacheFile(NULL, path.UTF8Characters,
                                      pathChar, URLOSTRM_GETNEWESTVERSION,
                                      0, NULL);
  if (hr == S_OK) {
    // If the file type isn't jpg and bmp, we need to convert.
    if (lstrcmpi(file_extension, ".jpg") != 0 &&
        lstrcmpi(file_extension, ".bmp") != 0) {

      if (!ConvertToJPEG(&pathChar)) {
        npfuncs_->setexception(scriptable_object_, "Cannot convert image.");
        return false;
      }
    }
    
    // Set the registry to apply the wallpaper styles and tiles.
    SetRegistry(tileInt, styleInt);
    if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, pathChar, 
        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
      return true;
    }
    else {
      int error = GetLastError();
      char errorString[1024];
      sprintf(errorString, "Cannot set wallpaper. Code:  %d Path: %s",
              error, pathChar);
      Debug(errorString);
    }
  }
  npfuncs_->setexception(scriptable_object_, "Cannot set wallpaper.");
  return false;
}

void DesktopService::Debug(char* message) {
  // Get window object.
  NPObject* window = NULL;
  npfuncs_->getvalue(npp_, NPNVWindowNPObject, &window);

  // Get console object.
  NPVariant consoleVar;
  NPIdentifier id = npfuncs_->getstringidentifier("console");
  npfuncs_->getproperty(npp_, window, id, &consoleVar);
  NPObject* console = NPVARIANT_TO_OBJECT(consoleVar);

  // Get the debug object.
  NPVariant debugVar;
  id = npfuncs_->getstringidentifier("debug");

  // Invoke the call with the message!
  NPVariant type;
  STRINGZ_TO_NPVARIANT(message, type);
  NPVariant args[] = { type };
  NPVariant voidResponse;
  npfuncs_->invoke(npp_, console, id, args, sizeof(args) / sizeof(args[0]), &voidResponse);

  // Cleanup all allocated objects, otherwise, reference count and
  // memory leaks will happen.
  npfuncs_->releaseobject(window);
  npfuncs_->releasevariantvalue(&consoleVar);
  npfuncs_->releasevariantvalue(&debugVar);
}

void DesktopService::SetRegistry(int tileInt, int styleInt) {
  char   subKey[] = "Control Panel\\Desktop";
  PRBool result = PR_FALSE;
  DWORD  dwDisp = 0;
  HKEY   key;
  // Try to create/open a subkey under HKLM.
  DWORD rc = RegCreateKeyEx(HKEY_CURRENT_USER, subKey, 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, &dwDisp );
  if (rc == ERROR_SUCCESS) {
    unsigned char tile[2];
    unsigned char style[2];
    tile[0] = tileInt + '0';
    style[0] = styleInt + '0';
    tile[1] = '\0';
    style[1] = '\0';
    RegSetValueEx(key, "TileWallpaper", 0, REG_SZ, tile, sizeof(tile));
    RegSetValueEx(key, "WallpaperStyle", 0, REG_SZ, style, sizeof(style));
  }
}

int DesktopService::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
   UINT  num = 0;  // Number of image encoders.
   UINT  size = 0; // Size of the image encoder array in bytes.

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j) {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 ) {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

bool DesktopService::ConvertToJPEG(char** path) {
  // Append jpg to the file for saving. (preserve space for extra chars)
  size_t path_len = strlen(*path);
  char* renamedChar = (char*)npfuncs_->memalloc(path_len + 5);
  strcpy(renamedChar, *path);
  strcat(renamedChar, ".jpg");

  // Create the file stream to write on.
  IStream* stream;
  HRESULT hr = SHCreateStreamOnFile(renamedChar,
      STGM_READWRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE, &stream);

  // Startup GDI+
  Status stat;
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // Prepare the JPEG encoder.
  CLSID jpgClsid;
  GetEncoderClsid(L"image/jpeg", &jpgClsid);

  // Save the file to the stream.
  bool success = true;
  WCHAR* tempPath = (WCHAR*)npfuncs_->memalloc(path_len + 1);
  MultiByteToWideChar(0, 0, *path, -1, tempPath, path_len + 1);
  Bitmap* bitmap = new Bitmap(tempPath);
  stat = bitmap->Save(stream, &jpgClsid, NULL);
  if (stat != Ok) {
    int error = GetLastError();
    char errorString[1024];
    sprintf(errorString, "Cannot save image. Code: %d|%d Path: %s",
            stat, error, path);
    Debug(errorString);
    success = false;
  }
  else {
    *path = renamedChar;
  }

  // Cleanup.
  GdiplusShutdown(gdiplusToken);
  stream->Release();
  
  return success;
}

}