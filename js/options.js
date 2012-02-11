// Extensions pages can all have access to the bacground page.
var bkg = chrome.extension.getBackgroundPage();

// When the DOM is loaded, make sure all the saved info is restored.
window.addEventListener('load', onLoad, false);

/**
 * When the options window has been loaded.
 */
function onLoad() {
  onRestore();
  $('button-close').addEventListener('click', onClose, false);
  $('release-notes').addEventListener('click', onReleaseNotes, false);
}

/**
 * When release notes is clicked.
 */
function onReleaseNotes() {
  bkg.openSingletonPage(chrome.extension.getURL('updates.html'));
}

/**
 * When the options window is closed;
 */
function onClose() {
  window.close();
}

/**
 * Restore all options.
 */
function onRestore() {
  // Restore settings.
  $('version').innerHTML = ' (v' + bkg.settings.version + ')';
  
  // Debug
  var debugElement = $('debug');
  debugElement.addEventListener('click', function(e) {
    bkg.settings.debug = debugElement.checked;
  });
  debugElement.checked = bkg.settings.debug;
  
  // Opt out
  var optElement = $('opt_out');
  optElement.addEventListener('click', function(e) {
    bkg.settings.opt_out = optElement.checked;
  });
  optElement.checked = bkg.settings.opt_out;
  
  // Add different positions.
  var positionElement = $('position')
  positionElement.add(createPositionOption('Stretch'));
  positionElement.add(createPositionOption('Center'));
  positionElement.add(createPositionOption('Tile'));
  if (bkg.isWindows7()) {
    positionElement.add(createPositionOption('Fill'));
    positionElement.add(createPositionOption('Fit'));
  }
  positionElement.value = bkg.settings.position;
  positionElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    bkg.settings.position = value;
  });
  
  // Restore the user interface options and its note.
  var userInterfaceElement = $('user_interface');
  userInterfaceElement.value = bkg.settings.user_interface;
  userInterfaceElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    setUserInterfaceNote(value);
    bkg.settings.user_interface = value;
  }, false);
  setUserInterfaceNote(bkg.settings.user_interface);

  // Whitelisted
  var whitelistedList = bkg.settings.whitelisted;
  var whitelistedElement = $('whitelisted_list');
  for (var i = 0; i < whitelistedList.length; i++) {
    whitelistedElement.add(new Option(whitelistedList[i]));
  }
  whitelistedElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    bkg.settings.whitelisted = value;
  });

  // Blacklisted
  var blacklistedList = bkg.settings.blacklisted;
  var blacklistedElement = $('blacklisted_list');
  for (var i = 0; i < blacklistedList.length; i++) {
    blacklistedElement.add(new Option(blacklistedList[i]));
  }
  blacklistedElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    bkg.settings.blacklisted = value;
  });
}

/**
 * Create Position Option
 *
 * @param {string} name The name of the element.
 * @returns {HTMLElement} The option.
 */
function createPositionOption(name) {
  var option = document.createElement('option');
  option.text = name;
  option.value = name.toLowerCase();
  return option;
}

/**
 * Changes the note for the userinterface, to help and assist the user what
 * the options really mean.
 *
 * @param {string} type The type of message that needs to change.
 */
function setUserInterfaceNote(type) {
  var message = 'Error, type does not exist.';
  if (type == 'overlay') {
    message = 'Creates an overlay preview within the tab itself.';
  }
  else if (type == 'none') {
    message = 'No preview, automatically sets the wallpaper with the default ' +
              'position.';
  }
  $('user_interface_note').innerHTML = message;
}