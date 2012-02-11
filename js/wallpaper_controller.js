/**
 * Main controller for the extension.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
WallpaperController = function() {
  this.onExtensionLoad();

  // Keep track if the operating system is Windows 7. We do special stuff.
  this.win7 = null;
  
  // Controllers for modifying state.
  this.tabRemovalController = new TabRemovalController(this);
  this.contextMenuController = new ContextMenuController(this);
  
  // Services we are using.
  this.extensionService = new ExtensionService(this);
};

/**
 * Initialization routine.
 */
WallpaperController.prototype.init = function() {
  this.tabRemovalController.init();
  this.contextMenuController.init();
  this.extensionService.init();
  this.initializePlugin();
};

/**
 * Fires when extension first loads.
 */
WallpaperController.prototype.onExtensionLoad = function() {
  var currVersion = chrome.app.getDetails().version;
  var prevVersion = settings.version
  if (currVersion != prevVersion) {
    // Update the version incase we want to do something in future.
    settings.version = currVersion;
    // Check if we just installed this extension.
    if (typeof prevVersion == 'undefined') {
      this.onInstall();
    }
    else {
      this.onUpdate();
    }
  }
};

/**
 * Open a singleton page, which means, if a page already exists, it
 * just selects it.
 *
 * @param url The page which it will navigate to.
 */
WallpaperController.prototype.openSingletonPage = function(url) {
  chrome.windows.getCurrent(function(win) {
    chrome.tabs.getAllInWindow(win.id, function(tabs) {
      for (var i = 0; i < tabs.length; i++) {
        if (tabs[i].url.indexOf(url) == 0) {
          chrome.tabs.update(tabs[i].id, {selected: true});
          return;
        }
      }
      chrome.tabs.create({url: url});
    });
  });
};


/**
 * Hook for installation.
 */
WallpaperController.prototype.onInstall = function() {
  chrome.tabs.create({url: 'options.html'});
};

/**
 * Hook for updates.
 */
WallpaperController.prototype.onUpdate = function() {
  // Do not send updates if the user is opt'd out!
  if (!settings.opt_out) {
    //chrome.tabs.create({url: 'updates.html'});
  }
};

/**
 * Get an instance of a guaranteed living plugin. If its dead (undefined) then
 * bring it back from the dead by removing/adding it back to the DOM.
 */
WallpaperController.prototype.getPlugin = function() {
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
WallpaperController.prototype.initializePlugin = function(opt_plugin) {
  var plugin = opt_plugin || this.getPlugin();
  plugin.debug = settings.debug;
};

/**
 * Lets check if we are currently on Windows7 since we have new properties
 * to expose.
 *
 * @returns {boolean} True if operating system is indeed running on windows 7.
 */
WallpaperController.prototype.isWindows7 = function() {
  if (this.win7 == null) {
    var re = /Windows\sNT\s([\d\.]+)/;
    var match_array = navigator.appVersion.match(re);
    this.win7 = match_array[1] >= 6.1;
  }
  return this.win7;
};

/**
 * Inject initialization data into the preview.
 *
 * Once the overlay.js file has been injected successfully, data such as OS
 * version and preview image needs to be send to that page. This should be
 * lazy loaded because we don't know when the preview finished initializing
 * the Chrome listeners. 
 *
 * Some platforms vary in initialization speed making the extension listeners
 * very slow resulting in a odd race condition. By sending the preview init
 * event at the end, we will have more chance of survival.
 */
WallpaperController.prototype.onPreviewLoaded = function(tab) {
  // Inform the tab wheter the current operating system is Windows 7.
  // Extra features will be enabled such as FILL/FIT algorithms.
  chrome.tabs.sendRequest(tab, {
    method: 'IsWindows7',
    data: this.isWindows7() 
  });

  // Safely set the image for the previewer after everything has been loaded.
  chrome.tabs.sendRequest(tab, {
    method: 'SetImage',
    data: {
      image: this.contextMenuController.getImageCacheURL(),
      position: settings.position
    }
  });
};
