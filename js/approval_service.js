/**
 * Approval service which is in charge of authenticating the approvals coming
 * from the external extensions.
 */
ApprovalService = function() {
  this.success = null;
  this.denied = null;
  this.recentlyApprovedMap = {};
};

ApprovalService.MIN_APPROVED_ACCESSED_TIME = 2000;
ApprovalService.MIN_ACCESSED_TIME_RETRIES = 3;

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
  chrome.management.get(extensionID, function(extensionInfo) {
    if (this.contains(settings.blacklisted, extensionID)) { // Blacklisted
      this.denied(extensionID, request, sendResponse);
    }
    else if (this.isAccessLimitReached(extensionInfo)) { // Limit Reached
      console.error('Access limit reached for ' + extensionInfo.name);
      this.denied(extensionID, request, sendResponse);
      return;
    }
    else if (this.contains(settings.whitelisted, extensionID)) { // White Listed
      this.success(extensionID, request, sendResponse);
    }
    else { // Needs to be verified.
      this.verify(extensionInfo, function(verified) {
        if (verified) {
          this.success(extensionID, request, sendResponse);
        }
        else {
          this.denied(extensionID, request, sendResponse);
        }
      }.bind(this));
    }
  }.bind(this));
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
 * @param {ExtensionInfo} extensionInfo Information about an installed extension.
 * @param {Function<object>} callback The callback to send back when verified.
 */
ApprovalService.prototype.verify = function(extensionInfo, callback) {
  var extensionID = extensionInfo.id;

  // Check if the approval dialog exists already for that extension.
  var approvalController = this.findApprovalController(extensionID);
  if (approvalController) {
    // TODO: Do some rate limiter so we can limit consecutive requests from the same extension.
    //       We would need this because an extension could be bad and abuse it, so we need to
    //       deal with that case and block it directly.
    callback(false); 
    return;
  }

  // Check if we already have this view opened.
  chrome.windows.create({
    url: 'approval.html#' + extensionID, type: 'popup', width: 600, height: 275
  }, function(win) {
    // Lazy load it, wait 100ms. Sometimes this callback doesn't work.
    setTimeout(function() {
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
          this.persistApproval('blacklisted', extensionInfo);
          callback(false);
        }
        else if (state === 'yes') {
          this.persistApproval('whitelisted', extensionInfo);
          callback(true);
        }
        else {
          callback(false);
        }
      }.bind(this));
    }.bind(this), 100);
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

/**
 * Persists the extensionInfo to the approval bucket based on the type.
 *
 * @param {string} type The approval type, either blacklisted or whitelisted.
 * @param {ExtensionInfo} extensionInfo Information about an installed extension.
 */
ApprovalService.prototype.persistApproval = function(type, extensionInfo) {
  if (!type.match('whitelisted|blacklisted')) {
    return;
  }
  var db = settings[type];
  db.push(extensionInfo.id + ':' + extensionInfo.name.replace(/,/g, ' '));
  settings[type] = db;
};

/**
 * Checks the last accessed time.
 *
 * @param {ExtensionInfo} extensionInfo Information about an installed extension.
 */
ApprovalService.prototype.isAccessLimitReached = function(extensionInfo) {
  var extensionID = extensionInfo.id;

  // Fix structure up for new approvals.
  if (!this.recentlyApprovedMap[extensionID]) {
    this.recentlyApprovedMap[extensionID] = {};
  }
  
  // Default data.
  var lastAccessedTime = this.recentlyApprovedMap[extensionID].lastAccessedTime || null;
  var retryCount = this.recentlyApprovedMap[extensionID].retryCount || 0;
  var currentAccessedTime = new Date().getTime();

  // Update the recently access time so we keep track. This means even if the user
  // tries to hammer, it will reset the timer so it will always be hammered.
  this.recentlyApprovedMap[extensionID].lastAccessedTime = currentAccessedTime;
  
  // Check if we havn't yet responded for this request treat it differently since we need
  // to populate the initial map. We are not retrying, so reset it.
  if (!lastAccessedTime) {
    this.recentlyApprovedMap[extensionID].retryCount = 0;
    return false;
  }

  // Check if we elapsed the recommended time constraints.
  if ((currentAccessedTime - lastAccessedTime) < ApprovalService.MIN_APPROVED_ACCESSED_TIME) {
    retryCount += 1;
    this.recentlyApprovedMap[extensionID].retryCount = retryCount;
    
    // Check whether to block this request since they are doing it way to fast. If so, blacklist it!
    if (retryCount > ApprovalService.MIN_ACCESSED_TIME_RETRIES) {
      this.persistApproval('blacklisted', extensionInfo);
    }
    return true;
  }

  // Everything okay, limit not reached so reset retry count.
  this.recentlyApprovedMap[extensionID].retryCount = 0;
  return false;
};