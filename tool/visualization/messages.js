var status = require("./status.js")
const ipcRenderer = require('electron').ipcRenderer

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

ipcRenderer.on('stream-start', function(event, data) {
  status.SetMessage("Collecting data");
})

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})