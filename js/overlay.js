// Copyright 2011 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

function showDialog() {
  // Unique ID to differentiate this content script from the rest of the web.  
  var UNIQUE_OVERLAY_ID = 'crx_wlp_iframe';

  // Check if the overlay is visible (this should never happen since we are
  // creating a modal dialog). Just for safe keeping.
  var overlayDOM = document.getElementById(UNIQUE_OVERLAY_ID);
  if (overlayDOM) {
    overlayDOM.parentNode.removeChild(overlayDOM);
  }

  // Create an overlay div with the unique id.
  overlayDOM = document.createElement('iframe');
  overlayDOM.setAttribute('id', UNIQUE_OVERLAY_ID);
  overlayDOM.setAttribute('src', chrome.extension.getURL('preview.html'));
  overlayDOM.setAttribute('frameBorder', '0');
  overlayDOM.setAttribute('width', '99.90%');
  overlayDOM.setAttribute('height', '100%');
  overlayDOM.setAttribute('style', 'position: absolute; top: 0; overflow: hidden; z-index: 99999');
  document.body.appendChild(overlayDOM);
}

// Hack since for some reason the extension system doesn't respond correctly
// when the iframe has width and height that are 100%. Some reason the listeners
// do not work. But when I execute the script again (second time) the listeners
// listen correctly. For now, this hack is to make sure it is visible at all 
// times. The downside, we render the iframe twice. Add / Remove / Add.
showDialog();
showDialog();