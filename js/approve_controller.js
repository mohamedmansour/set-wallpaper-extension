/**
 * Approver Controller, manages the state of the callback so it
 * can send a notification back to the consumer.
 *
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com)
 */
ApproveController = function() {
  this.callback = null;
};

/**
 * Initializes the button events.
 */
ApproveController.prototype.init = function() {
  $('yes').addEventListener('click', this.onYes.bind(this), false);
  $('no').addEventListener('click', this.onNo.bind(this), false);
  $('block').addEventListener('click', this.onBlock.bind(this), false);
};

/**
 * Sets the response listener for the callback for this approval.
 */
ApproveController.prototype.setResponseListener = function(extensionInfo, callback) {
  $('description').innerText = extensionInfo.description;
  $('name').innerText = extensionInfo.name;
  $('icon').src = extensionInfo.icons[extensionInfo.icons.length - 1].url;
  this.callback = callback;
};

/**
 * Fired when pressed YES.
 */
ApproveController.prototype.onYes = function() {
  this.callback('YES');
  window.close();
};

/**
 * Fired when pressed NO.
 */
ApproveController.prototype.onNo = function() {
  this.callback('NO');
  window.close();
};

/**
 * Fired when pressed BLOCK.
 */
ApproveController.prototype.onBlock = function() {
  this.callback('BLOCK');
  window.close();
};