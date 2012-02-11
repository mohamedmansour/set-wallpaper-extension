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
 * Fired when an external request came in.
 */
ExternalService.prototype.onRequestExternalListener = function(request, sender, sendResponse) {
  var extensionID = sender.id;
  chrome.management.get(extensionID, function(extensionInfo) {
    
  });
};