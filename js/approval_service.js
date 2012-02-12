/**
 * Approval service which is in charge of authenticating the approvals coming
 * from the external extensions.
 */
ApprovalService = function() {
  this.success = null;
  this.denied = null;
};

/**
 * Initializes the service.
 *
 * @param {Function} successCallback when something has been approved or whitelisted.
 * @param {Function} deniedCallback when something has been denied or blacklisted.
 */
ApprovalService.prototype.init = function(successCallback, deniedCallback) {
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
ApprovalService.prototype.onRequestExternalListener = function(request, sender, sendResponse) {
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
ApprovalService.prototype.contains = function(list, extensionID) {
  var found = false;
  list.some(function(elt) {
    var i = elt.indexOf(':');
    found = (elt.substring(0, i) == extensionID);
    return found;
  });
  return found;
};

/**
 * Verifies whether the extension could have access. It queries the extension management
 * store to get its meta data, and then opens up an approval window for manual intervention.
 * You cannot have more than one window opened at the same time, if you do, then it will
 * result in AccessDenied state for the response.
 *
 * @param {string} extensionID The extension ID.
 * @param {Function<object>} callback The callback to send back when verified.
 */
ApprovalService.prototype.verify = function(extensionID, callback) {
  // Check if the approval dialog exists already for that extension.
  var approvalController = this.findApprovalController(extensionID);
  if (approvalController) {
    // TODO: Do some rate limiter so we can limit consecutive requests from the same extension.
    //       We would need this because an extension could be bad and abuse it, so we need to
    //       deal with that case and block it directly.
    callback(false); 
    return;
  }
  chrome.management.get(extensionID, function(extensionInfo) {
    // Check if we already have this view opened.
    chrome.windows.create({
      url: 'approval.html#' + extensionID, type: 'popup', width: 600, height: 275
    }, function(win) {
      // Make sure the dialog has opened, otherwise send a failure response since something
      // clearly went wrong, maybe the IPC connection, they need to resend.
      var approvalController = this.findApprovalController(extensionID);
      if (!approvalController) {
        console.error('Cannot find approval controller for extension ' + extensionID);
        callback(false);
        return;
      }

      // Send extension info payload to that view and when an approval has been processed
      // send a response back to the initiator.
      approvalController.setResponseListener(extensionInfo, function(state) {
        if (state === 'block') {
          var blacklist = settings.blacklisted;
          blacklist.push(extensionID + ':' + extensionInfo.name.replace(/,/g, ' '));
          settings.blacklisted = blacklist;
          callback(false);
        }
        else if (state === 'yes') {
          var whitelist = settings.whitelisted;
          whitelist.push(extensionID + ':' + extensionInfo.name.replace(/,/g, ' '));
          settings.whitelisted = whitelist;
          callback(true);
        }
        else {
          callback(false);
        }
      });
    }.bind(this));
  }.bind(this));
};

/**
 * Finds a visible approval window from the given extensionID. If it doesn't exist
 * it will just return null. If it does exist, it will return its controller.
 *
 * @param {string} extensionID The extension ID.
 * @return {ApprovalController} The approval controller if found, otherwise just null.
 */
ApprovalService.prototype.findApprovalController = function(extensionID) {
  var viewIndex;
  var views = chrome.extension.getViews({type: 'tab'});
  for (viewIndex in views) {
    var obj = views[viewIndex];
    if (obj.location.pathname === '/approval.html' && obj.location.hash === '#' + extensionID) {
      return obj.controller;
    }
  }
  return null;
};