/**
 * Manages the context menu for the extension.
 */
ContextMenuController = function(controller) {
  this.controller = controller;
  this.imageCacheURL = null;
};

/**
 * Creates the context menu to set wallpaper.
 */
ContextMenuController.prototype.init = function() {
  chrome.contextMenus.create({
    title: 'Set image as wallpaper',
    contexts: ['image'],
    onclick: this.onImageMenuClicked.bind(this)
  });
};

/**
 * Callback when the context menu was clicked.
 *
 * @param {Object} info The context menu info.
 * @param {Object<Tab>} tab The tab that it was triggered.
 */
ContextMenuController.prototype.onImageMenuClicked = function(info, tab) {
  if (settings.user_interface == 'none') {
    // No interface present, just set the wallpaper directly with the
    // stored position in options.
    this.controller.getPlugin().setWallpaper(info.srcUrl, PositionEnum.valueOf(settings.position));
  }
  else { // overlay
    // Save the URL that was just right clicked. There is no way to transfer
    // that image directly without risking an odd race condition. There is no
    // guarantee that the iframe being loaded will be fast enough to handle
    // random injection. See doc onPreviewLoaded.
    this.imageCacheURL = info.srcUrl;

    // Inject the iframe previewer to the current DOM.
    chrome.tabs.executeScript(tab.id, {file: '/js/overlay.js'});
  }
};

/**
 * The context menu's image source.
 *
 * @return {string} The image URL.
 */
ContextMenuController.prototype.getImageCacheURL = function() {
  return this.imageCacheURL;
};