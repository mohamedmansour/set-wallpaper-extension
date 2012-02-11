/**
 * Approver service which is in charge of authenticating the approvals coming
 * from the external extensions.
 */
ApproverService = function() {
  this.success = null;
  this.denied = null;
};

/**
 * Initializes the service.
 *
 * @param {Function} successCallback when something has been approved or whitelisted.
 * @param {Function} deniedCallback when something has been denied or blacklisted.
 */
ApproverService.prototype.init = function(successCallback, deniedCallback) {
  this.success = successCallback;
  this.denied = deniedCallback;
  chrome.extension.onRequestExternal.addListener(this.onRequestExternalListener.bind(this));
};

/**
 * Fired when an external request came in verify access if it belongs in the whitelist.
 *
 * @param {object} request The payload that contains the request obj.
 * @param {object} sender The initiator who sent this request.
 * @param {Function<object>} sendResponse The callback to send back.
 */
ApproverService.prototype.onRequestExternalListener = function(request, sender, sendResponse) {
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
 * Checks if the list contains the extensionID.
 *
 * @param {Array<string>} list The list of extension ids.
 * @param {string} extensionID The extension ID.
 */
ApproverService.prototype.contains = function(list, extensionID) {
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
ApproverService.prototype.verify = function(extensionID, callback) {
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