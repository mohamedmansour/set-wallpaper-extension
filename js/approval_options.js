/**
 * Approval option component. That shows two list boxes for white list
 * and black list.
 *
 * Notes:
 *   Make sure the "init" is called when the DOM has been loaded,
 *   Make sure the "bindUI" is called when you build the UI.
 *   Make sure you have something like this in the HTML:
 *
 *     <dl>
 *       <dt>Whitelisted <br>(Allowed to set wallpaper):</dt>
 *       <dd>
 *         <select id="whitelisted_list" size="8"></select>
 *         <button id="whitelisted-list-remove">Remove</button>
 *         <button id="whitelisted-list-remove-all">Remove All</button>
 *         <div class="clear"></div>
 *       </dd>
 *         <dt>Blacklisted <br>(NOT allowed to set wallpaper):</dt>
 *       <dd>
 *         <select id="blacklisted_list" size="8"></select>
 *         <button id="blacklisted-list-remove">Remove</button>
 *         <button id="blacklisted-list-remove-all">Remove All</button>
 *         <div class="clear"></div>
 *       </dd>
 *     </dl>
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com) 
 */
ApprovalOptions = function() {
};
ApprovalOptions.BLACKLISTED_ID = 'blacklisted';
ApprovalOptions.WHITELISTED_ID = 'whitelisted';

/**
 * Initializes the events for this approver option.
 */
ApprovalOptions.prototype.init = function() {
  $(ApprovalOptions.WHITELISTED_ID +'-list-remove').addEventListener('click', this.onListRemove.bind(this), false);
  $(ApprovalOptions.WHITELISTED_ID +'-list-remove-all').addEventListener('click', this.onListRemoveAll.bind(this), false);
  $(ApprovalOptions.BLACKLISTED_ID +'-list-remove').addEventListener('click', this.onListRemove.bind(this), false);
  $(ApprovalOptions.BLACKLISTED_ID +'-list-remove-all').addEventListener('click', this.onListRemoveAll.bind(this), false);
};

/**
 * Binds the UI to the data from the backend.
 */
ApprovalOptions.prototype.bindUI = function() {
  // Whitelisted
  var whitelistedList = bkg.settings.whitelisted;
  var whitelistedElement = $(ApprovalOptions.WHITELISTED_ID + '_list');
  for (var i = 0; i < whitelistedList.length; i++) {
    var item = whitelistedList[i];
    whitelistedElement.add(new Option(item.substring(item.indexOf(':'))));
  }
  whitelistedElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    bkg.settings.whitelisted = value;
  });

  // Blacklisted
  var blacklistedList = bkg.settings.blacklisted;
  var blacklistedElement = $(ApprovalOptions.BLACKLISTED_ID + '_list');
  for (var i = 0; i < blacklistedList.length; i++) {
    var item = blacklistedList[i];
    blacklistedElement.add(new Option(item.substring(item.indexOf(':'))));
  }
  blacklistedElement.addEventListener('change', function(e) {
    var value = this.options[this.selectedIndex].value;
    bkg.settings.blacklisted = value;
  });
};

/**
 * Fired when a single item in the list is removed.
 */
ApprovalOptions.prototype.onListRemove = function(e) {
  var id = this.getListType(e);
  var list = $(id + '_list');
  if (list.selectedIndex != -1) {
    var data = bkg.settings[id];
    data.splice(list.selectedIndex, 1);
    bkg.settings[id] = data;
    list.remove(list.selectedIndex);
  }
  list.selectedIndex = list.length - 1;
};

/**
 * Fired when all the items in the list are removed.
 */
ApprovalOptions.prototype.onListRemoveAll = function(e) {
  var id = this.getListType(e);
  bkg.settings[id] = [];
  var list = $(id + '_list');
  while (list.length != 0) {
    list.remove();
  }
};

/**
 * From the given element, try getting the list type identifier.
 *
 * @return {string} The ID of the list.
 */
ApprovalOptions.prototype.getListType = function(e) {
  var id = ApprovalOptions.BLACKLISTED_ID;
  if (e.target.id.indexOf(ApprovalOptions.WHITELISTED_ID) == 0) {
    id = ApprovalOptions.WHITELISTED_ID;
  }
  return id;
};
