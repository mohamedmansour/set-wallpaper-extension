/**
 * Wallpaper service so consumers can change wallpapers on the fly.
 */
ExternalService = function(controller) {
  this.controller = controller;
  this.approvalService = new ApprovalService();
};

ExternalService.URL_PATTERN = new RegExp(/[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/gi);

/**
 * Initializes the service.
 */
ExternalService.prototype.init = function() {
  this.approvalService.init(this.success.bind(this), this.denied.bind(this));
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
    if (!PositionEnum.isValidOrdinal(position)) {
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