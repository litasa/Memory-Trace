const ipcRenderer = require('electron').ipcRenderer

connected = false;

window.onload = function() {
  initChart();
  }

ipcRenderer.once('server-init', function(event, serverData) {
  server = {};
  server.address = serverData.address;
  server.port = serverData.port;

  console.log('address: ' + server.address + ' port: ' + server.port);
})

ipcRenderer.once('connection-established', function() {
  connected = true;
})

ipcRenderer.on('event-done', function(event, data) {
  console.log('data recieved: ')
})
