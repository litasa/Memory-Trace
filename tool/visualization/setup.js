connected = false;
server = null;

var enc = require('..\\build\\Release\\Encryption');
var Visualization = require("./visualization.js")
Window.decoder = enc.Decoder();
Window.visualization_enabled = true;

window.onload = function() {
  Visualization.initChart();
  setTimeout(function() {
    sendToServer('please-connect', {addr: list});
  },2000);
  setInterval(Visualization.setUpdate,500, Visualization);
}

ipcRenderer.on('server-init', function(event, serverData) {
  server = {};
  server.address = serverData.address;
  server.port = serverData.port;
  Window.server = server;
  console.log("recieved from server.js")
  console.log('address: ' + server.address + ' port: ' + server.port);
  var text = document.createTextNode('address: ' + server.address + ' port: ' + server.port);
  document.getElementById("connect-message").appendChild(text);
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