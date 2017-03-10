connected = false;
server = null;

var enc = require('..\\build\\Release\\Encryption');
Window.decoder = enc.Decoder();
Window.visualization_enabled = true;

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
  Window.server = server;
  console.log("recieved from server.js")
  console.log('address: ' + server.address + ' port: ' + server.port);
})

ipcRenderer.on('connection-established', function() {
  Window.started = true;
  console.log("connection entered")
})

ipcRenderer.on('connection-closed', function() {
  Window.started = false;
  console.log("connection closed")
})

ipcRenderer.on('event-done', function(event, data) {
  console.log("data recieved");
})