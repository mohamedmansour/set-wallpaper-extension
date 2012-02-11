ApproveController = function() {
};

ApproveController.prototype.init = function() {
  chrome.extension.onRequest.addListener(this.onApproveInit.bind(this));
};

ApproveController.prototype.onApproveInit = function(request, sender, sendResponse) {
  if (request.service == 'ApprovalRequester') {
    var data = request.data;
    $('description').innerText = data.description;
    $('name').innerText = data.name;
    $('icon').src = data.icons[data.icons.length - 1].url;
  }
};

ApproveController.prototype.onVerificationRequest = function(msg) {
  console.log(extensionInfo);
  callback(true);
};