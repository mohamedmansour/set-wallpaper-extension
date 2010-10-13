// Copyright 2010 Mohamed Mansour. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.


/**
 * Positions for different wallpaper layouts.
 * 
 * @enum {Object<number, number>}
 */
PositionEnum = {
  CENTER  : 0,
  TILE    : 1,
  STRETCH : 2
};

/**
 * Converts the string representation of the enum to its value.
 *
 * @param {string} val The enum name as a text.
 * @return {Object<PositionEnum>} the enum value.
 */
PositionEnum.valueOf = function(val)
{
  return PositionEnum[val.toUpperCase()];
};