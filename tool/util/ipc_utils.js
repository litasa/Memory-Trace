const ipcRenderer = require('electron').ipcRenderer

sendTo = function(target, channel, data) {
    if(data === undefined) {
        data = {}
    }
    data.channel = channel;
    ipcRenderer.send(target,data);
}

sendToServer = function(channel, data) {
  sendTo('to-server', channel, data);
}

sendToChart = function(channel, data) {
  sendTo('to-chart', channel, data);
}

sendToInternal = function(channel,data) {
    sendTo('internal-server', channel, data);
}