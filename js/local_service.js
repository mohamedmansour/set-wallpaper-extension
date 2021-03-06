/**
 * Extension service that defines the messaging service from the entire extension.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
LocalService = function(controller) {
  this.controller = controller;
};

/**
 * Initializes the service.
 */
LocalService.prototype.init = function() {
  chrome.extension.onRequest.addListener(this.onExtensionRequestListener.bind(this));
};

/**
 * Extension request listener, called from content scripts and the extension
 * pages, so we have one centralized location that we deal with Plugin related
 * features.
 */
LocalService.prototype.onExtensionRequestListener = function(req, sender, sendResponse) {
  if (req.method == 'SetWallpaper') {
    this.controller.getPluginService().setWallpaper(req.data.image, req.data.position);
    sendResponse({});
  }
  else if (req.method == 'GetSystemColor') {
    sendResponse({color: this.controller.getPluginService().getSystemColor()});
  }
  else if (req.method == 'OpenOptions') {
    this.controller.openSingletonPage(chrome.extension.getURL('/options.html'));
    sendResponse({});
  }
  else if (req.method == 'RemoveOverlay') {
    var code = 'document.body.removeChild(document.querySelector("iframe#crx_wlp_iframe"))';
    chrome.tabs.executeScript(sender.tab.id, {code: code});
  }
  else if (req.method == 'PreviewLoaded') {
    this.controller.onPreviewLoaded(sender.tab.id);
  }
};