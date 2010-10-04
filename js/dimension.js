// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * Dimension Object.
 * @param {number} width
 * @param {number} height
 * @constructor
 */
Dimension = function(width, height)
{
  this.setWidth(width);
  this.setHeight(height);
};

/**
 * Sets the width.
 * @param {number} val The value for the width.
 */
Dimension.prototype.setWidth = function(val)
{
  var intVal = parseFloat(val);
  if (isNaN(intVal)) {
    intVal = 100;
  }
  this.width = intVal;
};

/**
 * Sets the height.
 * @param {number} val The value for the height.
 */
Dimension.prototype.setHeight = function(val)
{
  var intVal = parseFloat(val);
  if (isNaN(intVal)) {
    intVal = 100;
  }
  this.height = intVal;
};

/**
 * toString overload
 * @return {string} The string representation of the object.
 */
Dimension.prototype.toString = function() 
{
  return this.width + " x " + this.height;
};