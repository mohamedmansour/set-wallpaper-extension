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
  STRETCH : 2, 
  FIT     : 3, /* Windows 7 and later only */
  FILL    : 4, /* Windows 7 and later only */
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

/**
 * Checks if the ordinal is a valid one and doesn't exceed the number
 * of positions for this enum.
 *
 * @param {number} ordinal The position within the map.
 * @return {boolean} True if it lies within the same bounds otherwise false.
 */
PositionEnum.isValidOrdinal = function(ordinal)
{
  return !(isNaN(ordinal) || ordinal < 0 || ordinal > 4)
};