/**
 * Wallpaper service so consumers can change wallpapers on the fly.
 */
ExternalService = function(controller) {
  this.controller = controller;
};

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
  sendResponse({status: true, message: 'WallpaperSet'});
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
  list.some(function(elt, idx) {
    found = (elt == extensionID);
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
        url: 'approve.html#' + extensionID,
        type: 'popup',
        width: 600,
        height: 275
      }, function(win) {
        chrome.extension.getViews({type: 'tab'}).some(function(obj) {
          if (obj.location.pathname === '/approve.html') {
            obj.controller.setResponseListener(extensionInfo, function(state) {
              if (state === 'BLOCK') {
                callback(false);
              }
              else if (state === 'YES') {
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