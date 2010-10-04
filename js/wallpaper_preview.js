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
 */
WallpaperPreview = function(canvasID, imageURL, opt_position)
{
  this.canvas = document.getElementById(canvasID);
  this.ctx = canvas.getContext('2d');
  this.imageURL = imageURL;
  this.imageBuffer = document.createElement('img');
  this.imageBuffer.src = imageURL;
  this.imageBuffer.onload = this._loadComplete.bind(this);
  this.position = opt_position || PositionEnum.STRETCH;
};

/**
 * Get the location where the Image was fetched.
 * @return {string} The URL of the image currently viewed.
 */
WallpaperPreview.prototype.getImageURL = function() 
{
  return this.imageURL;
};

/**
 * The current rendered position.
 * @return {Object<PositionEnum>} The position currently visible.
 */
WallpaperPreview.prototype.getPosition = function() 
{
  return this.position;
};

/**
 * Render the specific position requested.
 * @param {Object<PositionEnum>} position The position that needs to be rendered.
 * @param {boolean} opt_force A flag that denotes if we want to force the
 *                            rendering to override the caching paint sequences.
 */
WallpaperPreview.prototype.render = function(position, opt_force) 
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
  }
};

/**
 * Sets the background color of the canvas. 
 * This comes from the Native side, which picks up your color from Windows.
 * @param {string} hex The hexadecimal color of the background canvas color.
 */
WallpaperPreview.prototype.setCanvasBackground = function(hex)
{
  var background = '#' + hex;
  this.canvas.style.backgroundColor = background;
};

/**
 * Buffered image callback when finished loading.
 * @private
 */
WallpaperPreview.prototype._loadComplete = function()
{
  this.screenDimension = new Dimension(screen.width, screen.height);
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
 * Stretch Renderer, renderers the buffered image on everything visible.
 * @private
 */
WallpaperPreview.prototype._renderStretch = function() 
{
  this.position = PositionEnum.STRETCH;
  this._paint(0, 0, this.canvasDimension.width, this.canvasDimension.height);
};

/**
 * Center Renderer, renderers the buffered image in the absolute center.
 * @private
 */
WallpaperPreview.prototype._renderCenter = function()  
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
 * Tile Renderer, renderers the buffered image in a tile, side by side.
 * @private
 */
WallpaperPreview.prototype._renderTile = function() 
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
 * Helper function that paints the current image on the canvas.
 * @param {number} x The position on the x relative to top left.
 * @param {number} y The position on the y relative to top left.
 * @param {number} width The maximum width of what we are painting.
 * @param {number} height The maximum height of what we are painting.
 * @private
 */
WallpaperPreview.prototype._paint = function(x, y, width, height) 
{
  this.ctx.drawImage(this.imageBuffer, x, y, width, height);
};