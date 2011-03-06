// Extensions pages can all have access to the bacground page.
var bkg = chrome.extension.getBackgroundPage();

// When the DOM is loaded, make sure all the saved info is restored.
window.addEventListener('load', onLoad, false);

/**
 * When the options window has been loaded.
 */
function onLoad() {
  onRestore();
  $('button-save').addEventListener('click', onSave, false);
  $('button-close').addEventListener('click', onClose, false);
}

/**
 * When the options window is closed;
 */
function onClose() {
  window.close();
}

/**
 * Saves options to localStorage.
 */
function onSave() {
  // Save settings.
  bkg.settings.debug = $('debug').checked;
  bkg.settings.user_interface = $('user_interface').value;
  bkg.settings.position = $('position').value;
  bkg.settings.opt_out = $('opt_out').checked;
  
  // Update status to let user know options were saved.
  var info = $('info-message');
  info.style.display = 'inline';
  info.style.opacity = 1;
  setTimeout(function() {
    info.style.opacity = 0.0;
  }, 1000);
}

/**
 * Restore all options.
 */
function onRestore() {
  // Restore settings.
  $('version').innerHTML = ' (v' + bkg.settings.version + ')';
  $('debug').checked = bkg.settings.debug;
  $('opt_out').checked = bkg.settings.opt_out;
  
  // Add different positions.
  var positionElement = $('position')
  positionElement.add(createPositionOption('Stretch'));
  positionElement.add(createPositionOption('Center'));
  positionElement.add(createPositionOption('Tile'));
  if (bkg.isWindows7()) {
    positionElement.add(createPositionOption('Fill'));
    positionElement.add(createPositionOption('Fit'));
  }
  $('position').value = bkg.settings.position;
  
  // Restore the user interface options and its note.
  var userInterfaceElement = $('user_interface');
  userInterfaceElement.value = bkg.settings.user_interface;
  userInterfaceElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    setUserInterfaceNote(value);
  }, false);
  setUserInterfaceNote(bkg.settings.user_interface);
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
  else if (type == 'newtab') {
    message = 'A new tab page will open with the wallpaper preview screen.';
  }
  $('user_interface_note').innerHTML = message;
}