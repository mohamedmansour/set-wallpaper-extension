/**
 * Wallpaper service so consumers can change wallpapers on the fly.
 */
ExternalService = function(controller) {
  this.controller = controller;
};

ExternalService.URL_PATTERN = new RegExp(/[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/gi);

/**
 * Initializes the service.
 */
ExternalService.prototype.init = function() {
  chrome.extension.onRequestExternal.addListener(this.onRequestExternalListener.bind(this));
};

/**
 * Fired when an external request came in verify access if it belongs in the whitelist.
 *
 * @param {object} request The payload that contains the request obj.
 * @param {object} sender The initiator who sent this request.
 * @param {Function<object>} sendResponse The callback to send back.
 */
ExternalService.prototype.onRequestExternalListener = function(request, sender, sendResponse) {
  var extensionID = sender.id;
  if (!this.contains(settings.whitelisted, extensionID)) {
    if (this.contains(settings.blacklisted, extensionID)) {
      this.denied(extensionID, request, sendResponse);
    }
    else {
      this.verify(extensionID, function(verified) {
        if (verified) {
          this.success(extensionID, request, sendResponse);
        }
        else {
          this.denied(extensionID, request, sendResponse);
        }
      }.bind(this));
    }
  }
  else {
    this.success(extensionID, request, sendResponse);
  }
};

/**
 * Success access callback.
 *
 * @param {string} extensionID The extension ID.
 * @param {object} request The payload that contains the request obj.
 * @param {Function<object>} sendResponse The callback to send back.
 */
ExternalService.prototype.success = function(extensionID, request, sendResponse) {
  if (request.method === 'Ping') {
    sendResponse({status: true, message: 'Pong'});
  }
  else if (request.method === 'SetWallpaper' && request.data != 'undefined') {
    if (!request.data) {
      sendResponse({status: false, message: 'InvalidArguments'});
      return;
    }

    // Validate url.
    var url = request.data.url || '';
    if (!url.match(ExternalService.URL_PATTERN)) {
      sendResponse({status: false, message: 'InvalidURL'});
      return;
    }

    // Validate position.
    var position = parseInt(request.data.position) || 0;
    if (!position) {
      sendResponse({status: false, message: 'InvalidPosition'});
      return;
    }

    // Call the plugin service to set the wallpaper.
    this.controller.getPluginService().setWallpaper(request.data.url, request.data.position);
    sendResponse({status: true, message: 'WallpaperSet'});
  }
  else {
    sendResponse({status: false, message: 'InvalidRequest'});
  }
};

/**
 * Denied access callback.
 *
 * @param {string} extensionID The extension ID.
 * @param {object} request The payload that contains the request obj.
 * @param {Function<object>} sendResponse The callback to send back.
 */
ExternalService.prototype.denied = function(extensionID, request, sendResponse) {
  sendResponse({status: false, message: 'AccessDenied'});
};

/**
 * Checks if the list contains the extensionID.
 *
 * @param {Array<string>} list The list of extension ids.
 * @param {string} extensionID The extension ID.
 */
ExternalService.prototype.contains = function(list, extensionID) {
  var found = false;
  list.some(function(elt) {
    var i = elt.indexOf(':');
    found = (elt.substring(0, i) == extensionID);
    return found;
  });
  return found;
};

/**
 * Verifies whether the extension could have access.
 *
 * @param {string} extensionID The extension ID.
 * @param {Function<object>} callback The callback to send back when verified.
 */
ExternalService.prototype.verify = function(extensionID, callback) {
  chrome.management.get(extensionID, function(extensionInfo) {
    chrome.windows.create({
        url: 'approver.html#' + extensionID,
        type: 'popup',
        width: 600,
        height: 275
      }, function(win) {
        chrome.extension.getViews({type: 'tab'}).some(function(obj) {
          if (obj.location.pathname === '/approver.html') {
            obj.controller.setResponseListener(extensionInfo, function(state) {
              if (state === 'BLOCK') {
                var blacklist = settings.blacklisted;
                blacklist.push(extensionID + ':' + extensionInfo.name.replace(/,/g, ' '));
                settings.blacklisted = blacklist;
                callback(false);
              }
              else if (state === 'YES') {
                var whitelist = settings.whitelisted;
                whitelist.push(extensionID + ':' + extensionInfo.name.replace(/,/g, ' '));
                settings.whitelisted = whitelist;
                callback(true);
              }
              else {
                callback(false);
              }
            });
            return true;
          }
          return false;
        });
    });
  }.bind(this));
};