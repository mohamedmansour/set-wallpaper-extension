/**
 * Approval Controller, manages the state of the callback so it
 * can send a notification back to the consumer.
 *
 * @author Mohamed Mansour 2012 (http://mohamedmansour.com)
 */
ApprovalController = function() {
  this.callback = null;
};

/**
 * Initializes the button events.
 */
ApprovalController.prototype.init = function() {
  $('yes').addEventListener('click', this.onResponseClickListener.bind(this), false);
  $('no').addEventListener('click', this.onResponseClickListener.bind(this), false);
  $('block').addEventListener('click', this.onResponseClickListener.bind(this), false);
};

/**
 * Sets the response listener for the callback for this approval.
 */
ApprovalController.prototype.setResponseListener = function(extensionInfo, callback) {
  $('description').innerText = extensionInfo.description;
  $('name').innerText = extensionInfo.name;
  $('icon').src = extensionInfo.icons[extensionInfo.icons.length - 1].url;
  this.callback = callback;
};

/**
 * Fired when any button on the page was clicked.
 */
ApprovalController.prototype.onResponseClickListener = function(e) {
  this.callback(e.target.id);
  window.close();
};