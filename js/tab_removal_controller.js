/**
 * Keep track of the tab position for the previewer when opened.
 * So that it can close back to the original place.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
TabRemovalController = function(controller) {
  this.controller = controller;
  this.currid = null;
  this.newid = null;
};

/**
 * Initializese the listener to start listening on removals.
 */
TabRemovalController.prototype.init = function() {
  chrome.tabs.onRemoved.addListener(this.onTabRemovedListener.bind(this));
};

/**
 * Listens on tab removals. So we can get back to the tab that just opened.
 * If possible of course. Should handle moved tabs as well.
 */
TabRemovalController.prototype.onTabRemovedListener = function(tabId) {
  if (this.newid && tabId == this.newid) {
    chrome.tabs.update(this.currid, {selected: true})
  } else {
    this.newid = null; 
  }
};