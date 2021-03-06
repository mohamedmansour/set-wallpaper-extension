// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * Wallpaper Previewer that imitates what will happen in Windows.
 * @constructor
 * @param {string} canvasID The canvas ID for the DOM.
 * @param {string} imageURL The URL of the image to preview.
 * @param {Object<PositionEnum>} opt_position Optional position where the
 *                               default is STRETCH.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
PreviewRenderer = function(canvasID, imageURL, opt_position)
{
  this.ctx = document.getElementById(canvasID).getContext('2d');
  this.canvas = document.getElementById(canvasID); // Need to recall it, bug.
  this.imageURL = imageURL;
  this.imageBuffer = document.createElement('img');
  this.imageBuffer.src = imageURL;
  this.imageBuffer.onload = this._loadComplete.bind(this);
  this.position = (typeof opt_position == 'undefined') ?
      PositionEnum.STRETCH : opt_position;
};

/**
 * Get the location where the Image was fetched.
 * @return {string} The URL of the image currently viewed.
 */
PreviewRenderer.prototype.getImageURL = function() 
{
  return this.imageURL;
};

/**
 * The current rendered position.
 * @return {Object<PositionEnum>} The position currently visible.
 */
PreviewRenderer.prototype.getPosition = function() 
{
  return this.position;
};

/**
 * Render the specific position requested.
 * @param {Object<PositionEnum>} position The position that needs to be rendered.
 * @param {boolean} opt_force A flag that denotes if we want to force the
 *                            rendering to override the caching paint sequences.
 */
PreviewRenderer.prototype.render = function(position, opt_force) 
{
  // No need to re-render if we didn't request a new position.
  var force = opt_force || false;
  if (!force && this.position == position) {
    return;
  }
  
  // Clear the canvas, since a new rendering routine is happening.
  this.ctx.clearRect(0, 0, this.canvasDimension.width,
                     this.canvasDimension.height);

  // Figure out renderer routine to paint.
  switch (position) {
    case PositionEnum.TILE:
      this._renderTile();
      break;
    case PositionEnum.CENTER:
      this._renderCenter();
      break;
    case PositionEnum.STRETCH:
      this._renderStretch();
      break;
    case PositionEnum.FILL:
      this._renderFill();
      break;
    case PositionEnum.FIT:
      this._renderFit();
      break;
  }
};

/**
 * Sets the background color of the canvas. 
 * This comes from the Native side, which picks up your color from Windows.
 * @param {string} hex The hexadecimal color of the background canvas color.
 */
PreviewRenderer.prototype.setCanvasBackground = function(hex)
{
  var background = '#' + hex;
  this.canvas.style.backgroundColor = background;
};

/**
 * Buffered image callback when finished loading.
 * @private
 */
PreviewRenderer.prototype._loadComplete = function()
{
  this.screenDimension = new Dimension(screen.width, screen.height);
  this.canvas.height = this.canvas.width / (this.screenDimension.width /
      this.screenDimension.height);
  this.canvasDimension = new Dimension(this.canvas.width, this.canvas.height);
  this.imageDimension = new Dimension(this.imageBuffer.width,
                                       this.imageBuffer.height);
  this.factor = new Dimension(this.screenDimension.width /
                                   this.canvasDimension.width,
                               this.screenDimension.height /
                                   this.canvasDimension.height);
  this.render(this.position, true);
};

/**
 * Stretch Renderer, Stretch the image to cover the full screen. This can result
 * in distortion of the image as the image's aspect ratio is not retained.
 * @private
 */
PreviewRenderer.prototype._renderStretch = function() 
{
  this.position = PositionEnum.STRETCH;
  this._paint(0, 0, this.canvasDimension.width, this.canvasDimension.height);
};

/**
 * Center Renderer, Center the wallpaper image in its original size, filling the
 * remaining area with a solid background color if image is smaller than screen
 * or cropping image if image is larger.
 * @private
 */
PreviewRenderer.prototype._renderCenter = function()  
{
  this.position = PositionEnum.CENTER;
  
  // Scale the dimensions while constraining proportions.
  var width = this.imageDimension.width / this.factor.width;
  var height = this.imageDimension.height / this.factor.height;
  
  // Move the wallpaper to the center.
  var x = (this.canvasDimension.width - width) / 2;
  var y = (this.canvasDimension.height - height) / 2;
  this._paint(x, y, width, height);
};

/**
 * Tile Renderer, Tile the wallpaper image, starting in the upper left corner of
 * the screen. This uses the image in its original size.
 * @private
 */
PreviewRenderer.prototype._renderTile = function() 
{
  this.position = PositionEnum.TILE;
  
  // Scale the dimensions while constraining proportions.
  var width = this.imageDimension.width / this.factor.width;
  var height = this.imageDimension.height / this.factor.height;
  
  // Figure out how many photos we can tile.
  var maxX = Math.ceil((this.canvasDimension.width - width) / width) + 1;
  var maxY = Math.ceil((this.canvasDimension.height - height) / height) + 1;
  for (var i = 0; i < maxX; i++) {
    for (var j = 0; j < maxY; j++) {
      var x = i * width;
      var y = j * height;
      this._paint(x, y, width, height);
    }
  }
};

/**
 * Fill Renderer, Enlarge or shrink the image to fill the screen, retaining the
 * aspect ratio of the original image. If necessary, the image is padded either
 * on the top and bottom or on the right and left with the background color to
 * fill any screen area not covered by the image.
 * @private
 */
PreviewRenderer.prototype._renderFill = function() 
{
  this.position = PositionEnum.FILL;
  
  // Scale the dimensions while constraining proportions.
  var width = this.imageDimension.width / this.factor.width;
  var height = this.imageDimension.height / this.factor.height;
  
  // Lets check if it needs to enlarge or shrink.
  if (width < this.canvasDimension.width &&
      height < this.canvasDimension.height) { 
    // Enlarging.
    var widthDiff = this.canvasDimension.width - width;
    var heightDiff = this.canvasDimension.height - height;
    // Enlarging till the smaller side.
    if (widthDiff < heightDiff) {
      width = (width * this.canvasDimension.height) / height;
      height = this.canvasDimension.height;
    }
    else {
      height = (height * this.canvasDimension.width) / width;
      width = this.canvasDimension.width;
    }
  }
  else {
    // Shrinking.
    var widthDiff = width - this.canvasDimension.width;
    var heightDiff = height - this.canvasDimension.height;
    // Shrinking till the smaller side.
    if (widthDiff < heightDiff) {
      height = (height * this.canvasDimension.width) / width;
      width = this.canvasDimension.width;
    }
    else {
      width = (width * this.canvasDimension.height) / height;
      height = this.canvasDimension.height;
    }
  }
  
  // Move the wallpaper to the center.
  var x = (this.canvasDimension.width - width) / 2;
  var y = (this.canvasDimension.height - height) / 2;
  this._paint(x, y, width, height);
};

/**
 * Fit Renderer, Enlarge or shrink the image to fill the screen, retaining
 * the aspect ratio of the original image. If necessary, the image is cropped
 * either on the top and bottom or on the left and right as necessary to fit the
 * screen.
 * @private
 */
PreviewRenderer.prototype._renderFit = function() 
{
  this.position = PositionEnum.FIT;

  // Scale the dimensions while constraining proportions.
  var width = this.imageDimension.width / this.factor.width;
  var height = this.imageDimension.height / this.factor.height;
  
  // Lets check if it needs to enlarge or shrink.
  if (width < this.canvasDimension.width &&
      height < this.canvasDimension.height) { 
    // Enlarging.
    var widthDiff = this.canvasDimension.width - width;
    var heightDiff = this.canvasDimension.height - height;
    // Enlarging till the larger side.
    if (widthDiff > heightDiff) {
      width = (width * this.canvasDimension.height) / height;
      height = this.canvasDimension.height;
    }
    else {
      height = (height * this.canvasDimension.width) / width;
      width = this.canvasDimension.width;
    }
  }
  else {
    // Shrinking.
    var widthDiff = width - this.canvasDimension.width;
    var heightDiff = height - this.canvasDimension.height;
    // Shrink till the larger side.
    if (widthDiff > heightDiff) {
      height = (height * this.canvasDimension.width) / width;
      width = this.canvasDimension.width;
    }
    else {
      width = (width * this.canvasDimension.height) / height;
      height = this.canvasDimension.height;
    }
  }
  
  // Move the wallpaper to the center.
  var x = (this.canvasDimension.width - width) / 2;
  var y = (this.canvasDimension.height - height) / 2;
  this._paint(x, y, width, height);
};

/**
 * Helper function that paints the current image on the canvas.
 * @param {number} x The position on the x relative to top left.
 * @param {number} y The position on the y relative to top left.
 * @param {number} width The maximum width of what we are painting.
 * @param {number} height The maximum height of what we are painting.
 * @private
 */
PreviewRenderer.prototype._paint = function(x, y, width, height) 
{
  this.ctx.drawImage(this.imageBuffer, x, y, width, height);
};