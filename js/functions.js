/**
 * Short form for getting elements by id.
 * @param {string} id The id.
 */
function $(id) {
  return document.getElementById(id);
}

/**
 * Trim the results from spaces from the beginning and end
 * @return {string} the trimmed string.
 */
String.prototype.trim = function () {
    return this.replace(/^\s*/, '').replace(/\s*$/, '');
};