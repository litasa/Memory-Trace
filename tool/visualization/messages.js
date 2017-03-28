var status = require("./status.js")
const ipcRenderer = require('electron').ipcRenderer

ipcRenderer.on('server-init', function(event, server) {
  status.SetIpAddress(server.address,server.port);
})

ipcRenderer.on('connection-established', function(event, data) {
    console.log("opened connection")
  status.SetMessage("Collecting data");
})

ipcRenderer.on('connection-closed', function(event, data) {
    console.log("closed connection")
  status.SetMessage("Successfully collected data");
})

ipcRenderer.on('external-error', function(event, data) {
    status.SetErrorMessage("ERROR: " + data.msg)
})