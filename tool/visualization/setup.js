connected = false;
server = null;

window.onload = function() {
  initChart();
    setTimeout(function() {
    sendToServer('please-connect', {addr: list});
  },2000);
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
  //console.log('data recieved: ')
  //var arr = enc.Decoder.getMemoryAsArray();
  //console.log("got array of size: " + arr.length);
})