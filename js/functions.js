/**
 * Short form for getting elements by id.
 * @param {string} id The id.
 */
function $(id) {
  return document.getElementById(id);
}

/**
 * Function extension for binding the current scope.
 * @param {Object} scope the scope to bind the function that is extended.
 */
Function.prototype.bind = function(scope) {
  var _function = this;
  return function() {
    return _function.apply(scope, arguments);
  }
};

/**
 * Trim the results from spaces from the beginning and end
 * @return {string} the trimmed string.
 */
String.prototype.trim = function () {
    return this.replace(/^\s*/, '').replace(/\s*$/, '');
};