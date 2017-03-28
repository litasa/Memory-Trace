var status = require("./status.js")
const ipcRenderer = require('electron').ipcRenderer

ipcRenderer.on('server-init', function(event, server) {
  status.SetIpAddress(server.address,server.port);
})

ipcRenderer.on('stream-start', function(event, data) {
  status.SetMessage("Collecting data");
})

ipcRenderer.on('external-error', function(event, data) {
    status.SetErrorMessage("ERROR: " + data.msg)
})