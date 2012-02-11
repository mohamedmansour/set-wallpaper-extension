/**
 * Service that communicates directly with the plugin.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
PluginService = function(controller) {
  this.controller = controller;
};

/**
 * Initilzes the plugin.
 */
PluginService.prototype.init = function() {
  this.initializePlugin();
  settings.addListener('debug', this.onSettingChangedListener.bind(this));
};

/**
 * Get an instance of a guaranteed living plugin. If its dead (undefined) then
 * bring it back from the dead by removing/adding it back to the DOM.
 */
PluginService.prototype.getPlugin = function() {
  // Check if plugin somehow crashed, most likely due to some internal error.
  // Lets just reload it since some images might work.
  var plugin = document.getElementById('pluginobj');
  if (plugin.debug == 'undefined') {
    var parent = plugin.parentNode;;
    parent.removeChild(plugin);
    parent.appendChild(plugin);
    this.initializePlugin(plugin);
  }
  return plugin;
};

/**
 * Initialized Plugin with defaults.
 *
 * @param {Embed} opt_plugin Optional plugin to use instead.
 */
PluginService.prototype.initializePlugin = function(opt_plugin) {
  var plugin = opt_plugin || this.getPlugin();
  plugin.debug = settings.debug;
};

/**
 * Fired when a plugin setting was changed. We should update the object.
 */
PluginService.prototype.onSettingChangedListener = function(key, val) {
  this.getPlugin().debug = val;
};

/**
 * Access the systemColor native plugin call.
 */
PluginService.prototype.getSystemColor = function() {
  return this.getPlugin().systemColor();
};

/**
 * Access the setWallpaper native plugin call.
 */
PluginService.prototype.setWallpaper = function(imageURL, imageStyle) {
  return this.getPlugin().setWallpaper(imageURL, imageStyle);
};