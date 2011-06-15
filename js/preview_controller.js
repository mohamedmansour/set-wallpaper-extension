// Copyright 2011 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * Controlls the Preview Page.
 * @constructor
 */
PreviewController = function()
{
  this.preview = null;
};

/**
 * Bind preview listeners to the DOM to setup the renderer.
 */
PreviewController.prototype.bind = function()
{
  window.addEventListener('load', this.onLoad.bind(this));
  chrome.extension.onRequest.addListener(this.onMessage.bind(this));
};

/**
 * External extension request listeners message handling.
 */
PreviewController.prototype.onMessage = function(req, sender, sendResponse)
{
  if (req.method == 'SetImage') {
    // Startup the previewer to do its business!
    this.preview = new WallpaperPreview('crx_wlp_canvas', req.data.image,
                                         PositionEnum.valueOf(req.data.position));
    // Send a call natively to inform the operating system about its color.
    chrome.extension.sendRequest({method: 'GetSystemColor'},
                                 this.onSystemColorChange.bind(this));
    // Set the appropriate item selected.
    var i = $('crx_wlp_' + req.data.position + 'Button');
    i.className = 'selected';
  }
  else if (req.method = 'IsWindows7') {
    if (req.data) {
      this.setupWindows7Components();
    }
  }
  sendResponse({});
};

/**
 * System Color Change response.
 */
PreviewController.prototype.onSystemColorChange = function(res)
{
  this.preview.setCanvasBackground(res.color);
};


/**
 * Preview DOM loaded response.
 */
PreviewController.prototype.onLoad = function()
{
  var self = this;

  // Add button click event listeners.
  $('crx_wlp_stretchButton').addEventListener('click', function() {
    self.changeSelection(this, PositionEnum.STRETCH);
  }, false);
  $('crx_wlp_tileButton').addEventListener('click', function() {
    self.changeSelection(this, PositionEnum.TILE);
  }, false);
  $('crx_wlp_centerButton').addEventListener('click', function() {
    self.changeSelection(this, PositionEnum.CENTER);
  }, false);
  $('crx_wlp_fillButton').addEventListener('click', function() {
    self.changeSelection(this, PositionEnum.FILL);
  }, false);
  $('crx_wlp_fitButton').addEventListener('click', function() {
    self.changeSelection(this, PositionEnum.FIT);
  }, false);
  $('wallpaper_submit').addEventListener('click', function() {
    self.doSubmit();
  }, false);
  $('wallpaper_options').addEventListener('click', function() {
    self.doOptions();
  }, false);
  $('wallpaper_close').addEventListener('click', function() {
    self.doClose();
  }, false);

  chrome.extension.sendRequest({method: 'PreviewLoaded'});
};

/**
 * Change the selection of the wallpaper style.
 *
 * @param {HTMLElement} dom The current list item.
 * @param {Object<PositionEnum>} position The wallpaper position to set.
 */
PreviewController.prototype.changeSelection = function(dom, position)
{
  var lists = document.getElementsByTagName('li');
  for (var i in lists) {
    var list = lists[i];
    list.className = '';
  }
  dom.className = 'selected';
  this.preview.render(position);
};
  
/**
 * Submit a request to change the wallpaper.
 */
PreviewController.prototype.doSubmit = function()
{
  chrome.extension.sendRequest({
      method: 'SetWallpaper', 
      data: {
        image: this.preview.getImageURL(),
        position: this.preview.getPosition()
      }
  });
};

/**
 * View wallpaper extension options.
 */
PreviewController.prototype.doOptions = function()
{
   chrome.extension.sendRequest({method: 'OpenOptions'});
};

/**
 * Close IFrame Preview by sending a request back to the main extension.
 */
PreviewController.prototype.doClose = function()
{
  if (parent) {
    chrome.extension.sendRequest({method: 'RemoveOverlay'});
  }
  else {
    window.close();
  }
};

/**
 * Lets add Win7 specific positions and icons.
 */
PreviewController.prototype.setupWindows7Components = function()
{
  var fillButton = $('crx_wlp_fillButton');
  var fitButton = $('crx_wlp_fitButton');
  fillButton.style.display = 'inline-block';
  fitButton.style.display = 'inline-block';
};
