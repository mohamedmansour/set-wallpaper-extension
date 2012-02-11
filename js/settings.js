// Global localStorage persisted settings.
settings = {
  get version() {
    return localStorage['version'];
  },
  set version(val) {
    settings.notify('version', val);
    localStorage['version'] = val;
  },
  get debug() {
    var key = localStorage['debug'];
    return (typeof key == 'undefined') ? false : key === 'true';
  },
  set debug(val) {
    settings.notify('debug', val);
    localStorage['debug'] = val;
  },
  get user_interface() {
    var key = localStorage['user_interface'];
    return (typeof key == 'undefined') ? 'overlay' : key;
  },
  set user_interface(val) {
    settings.notify('user_interface', val);
    localStorage['user_interface'] = val;
  },
  get position() {
    var key = localStorage['position'];
    return (typeof key == 'undefined') ? 'stretch' : key;
  },
  set position(val) {
    settings.notify('position', val);
    localStorage['position'] = val;
  },
  get opt_out() {
    var key = localStorage['opt_out'];
    return (typeof key == 'undefined') ? true : key === 'true';
  },
  set opt_out(val) {
    settings.notify('opt_out', val);
    localStorage['opt_out'] = val;
  },
  get whitelisted() {
    var key = localStorage['whitelisted'];
    return (typeof key == 'undefined') ? [] : (key == '' ? [] : key.split(', '));
  },
  set whitelisted(val) {
    if (typeof val == 'object') {
      settings.notify('whitelisted', val);
      localStorage['whitelisted'] = val.sort().join(', ');
    }
  },
  get blacklisted() {
    var key = localStorage['blacklisted'];
    return (typeof key == 'undefined') ? [] : (key == '' ? [] : key.split(', '));
  },
  set blacklisted(val) {
    if (typeof val == 'object') {
      settings.notify('blacklisted', val);
      localStorage['blacklisted'] = val.sort().join(', ');
    }
  },
};

// Settings event listeners.
settings.listeners = {};
settings.notify = function(key, val) {
  var listeners = settings.listeners[key]
  if (listeners) {
    listeners.forEach(function(callback, index) {
      callback(key, val);
    });
  }
};
settings.addListener = function(key, callback) {
  if (!settings.listeners[key]) {
    settings.listeners[key] = [];
  }
  settings.listeners[key].push(callback);
};