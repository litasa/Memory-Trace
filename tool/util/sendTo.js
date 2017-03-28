const ipcRenderer = require('electron').ipcRenderer
var exports = module.exports = {};

var send = function(target, channel, data) {
    if(data === undefined) {
        data = {}
    }
    data.channel = channel;
    ipcRenderer.send(target, data);
}

exports.Server = function(channel, data) {
  send('to-server', channel, data);
}

exports.Chart = function(channel, data) {
  send('to-chart', channel, data);
}

exports.Internal = function(channel,data) {
    send('internal-server', channel, data);
}