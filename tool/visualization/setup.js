connected = false;
server = null;

var enc = require('..\\build\\Release\\Encryption');
Window.decoder = enc.Decoder();

window.onload = function() {
  initChart();
    setTimeout(function() {
    sendToServer('please-connect', {addr: list});
  },2000);
}

ipcRenderer.on('server-init', function(event, serverData) {
  server = {};
  server.address = serverData.address;
  server.port = serverData.port;

  console.log('address: ' + server.address + ' port: ' + server.port);
})

ipcRenderer.once('connection-established', function() {
  connected = true;
})

ipcRenderer.on('event-done', function(event, data) {
  console.log("data recieved");
})