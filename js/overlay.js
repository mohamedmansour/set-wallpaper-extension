// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// Unique ID to differentiate this content script from the rest of the web.  
var UNIQUE_OVERLAY_ID = 'fnpmapejgepknmmkciiclcloomblhopm';

// Check if the overlay is visible (this should never happen since we are
// creating a modal dialog). Just for safe keeping.
var overlayDOM = document.getElementById(UNIQUE_OVERLAY_ID);
if (overlayDOM) {
  overlayDOM.parentNode.removeChild(overlayDOM);
}

// Create an overlay div with the unique id.
overlayDOM = document.createElement('div');
overlayDOM.id = UNIQUE_OVERLAY_ID;

// Create the inner div which will be the modal.
var overlayInnerDIV = document.createElement('div');
overlayDOM.appendChild(overlayInnerDIV);
document.body.appendChild(overlayDOM);

// The User Interface.
var text = '' +
'  <div id="crx_wlp_area">' +
'    <div id="crx_wlp_preview">' +
'      <h1>Set Desktop Wallpaper</h1>' +
'      <canvas id="crx_wlp_canvas" width="500" height="375" ></canvas>' +
'      <div id="options">' +
'        <ul>' +
'          <li id="crx_wlp_stretchButton">Stretch</li>' +
'          <li id="crx_wlp_tileButton">Tile</li>' +
'          <li id="crx_wlp_centerButton">Center</li>' +
'          <li id="crx_wlp_fillButton" style="display:none">Fill</li>' +
'          <li id="crx_wlp_fitButton" style="display:none">Fit</li>' +
'        </ul>' +
'      </div>' +
'      <div id="crx_wlp_buttons">' +
'        <a class="green crx_wlp_button" id="crx_wlp_setWallpaper">Set wallpaper</a>' +
'        <a class="orange crx_wlp_button" id="crx_wlp_getOptions">Options</a>' +
'        <a class="yellow crx_wlp_button" id="crx_wlp_closePreview">Close</a>' +
'      </div>' +
'    </div>' +
'    <div id="crx_wlp_tweet" ><a href="http://twitter.com/mohamedmansour">' +
'      @mohamedmansour</a>' +
'    </div>' +
'  </div>';
         
overlayInnerDIV.innerHTML = text;
overlayDOM.style.visibility = 'visible';

// The preview implementor.
var preview = null;

// Close Button Click Listener.
document.getElementById('crx_wlp_closePreview').addEventListener('click', function(e) {
  overlayDOM.parentNode.removeChild(overlayDOM);
}, false);

// Set Wallpaper Click Listener.
document.getElementById('crx_wlp_setWallpaper').addEventListener('click', function(e) {
  var position = preview.getPosition();
  chrome.extension.sendRequest({
      method: 'SetWallpaper', 
      data: {
        image: preview.getImageURL(),
        position: position
      }
  });
}, false);

// Get Options Click Listener.
document.getElementById('crx_wlp_getOptions').addEventListener('click', function(e) {
  chrome.extension.sendRequest({method: 'OpenOptions'});
}, false);

// Position Click Listener.
var stretchButton = document.getElementById('crx_wlp_stretchButton');
var tileButton = document.getElementById('crx_wlp_tileButton');
var centerButton = document.getElementById('crx_wlp_centerButton');
var fillButton = document.getElementById('crx_wlp_fillButton');
var fitButton = document.getElementById('crx_wlp_fitButton');

// Stretch.
stretchButton.style.backgroundImage =
    'url(' + chrome.extension.getURL('/img/stretch.png') + ')';
stretchButton.addEventListener('click', function(e) {
  stretchButton.className = 'selected';
  tileButton.className = '';
  centerButton.className = '';
  fillButton.className = '';
  fitButton.className = '';
  preview.render(PositionEnum.STRETCH);
}, false);

// Tile.
tileButton.style.backgroundImage = 
    'url(' + chrome.extension.getURL('/img/tile.png') + ')';
tileButton.addEventListener('click', function(e) {
  stretchButton.className = '';
  tileButton.className = 'selected';
  centerButton.className = '';
  fillButton.className = '';
  fitButton.className = '';
  preview.render(PositionEnum.TILE);
}, false);

// Center.
centerButton.style.backgroundImage = 
    'url(' + chrome.extension.getURL('/img/center.png') + ')';
centerButton.addEventListener('click', function(e) {
  stretchButton.className = '';
  tileButton.className = '';
  centerButton.className = 'selected';
  fillButton.className = '';
  fitButton.className = '';
  preview.render(PositionEnum.CENTER);
}, false);


// Lets add Win7 specific positions.
function setupWindows7Components() {
  fillButton.style.display = 'inline-block';
  fitButton.style.display = 'inline-block';

  // Fill.
  fillButton.style.backgroundImage = 
      'url(' + chrome.extension.getURL('/img/fill.png') + ')';
  fillButton.addEventListener('click', function(e) {
    stretchButton.className = '';
    tileButton.className = '';
    centerButton.className = '';
    fillButton.className = 'selected';
    fitButton.className = '';
    preview.render(PositionEnum.FILL);
  }, false);

  // Fit.
  fitButton.style.backgroundImage = 
      'url(' + chrome.extension.getURL('/img/fit.png') + ')';
  fitButton.addEventListener('click', function(e) {
    stretchButton.className = '';
    tileButton.className = '';
    centerButton.className = '';
    fillButton.className = '';
    fitButton.className = 'selected';
    preview.render(PositionEnum.FIT);
  }, false);
}

// Listen for extension requests.
chrome.extension.onRequest.addListener(function(req, sender, sendResponse) {
  if (req.method == 'SetImage') {
    // Startup the previewer to do its business!
    preview = new WallpaperPreview('crx_wlp_canvas', req.data.image,
                                    PositionEnum.valueOf(req.data.position));
    // Send a call natively to inform the operating system about its color.
    chrome.extension.sendRequest({method: 'GetSystemColor'}, function(res) {
      preview.setCanvasBackground(res.color);
    });
    // Set the appropriate item selected.
    var i = document.getElementById('crx_wlp_' + req.data.position + 'Button');
    i.className = 'selected';
  }
  else if (req.method = 'IsWindows7') {
    if (req.data) {
      setupWindows7Components();
    }
  }
  sendResponse({});
});

