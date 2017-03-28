connected = false;
server = null;

var enc = require('..\\build\\Release\\Encryption');
var Visualization = require("./visualization.js")
var status = require("./status.js")
Window.decoder = enc.Decoder();
Window.visualization_enabled = true;

window.onload = function() {
  Visualization.initChart();
  setTimeout(function() {
    sendToServer('please-connect', {addr: list});
  },2000);
  setInterval(Visualization.setUpdate,500, Visualization);
}

ipcRenderer.on('server-init', function(event, server) {
  status.SetIpAddress(server.address,server.port);
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