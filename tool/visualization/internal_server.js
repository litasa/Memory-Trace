const net = require('net');

var list = '\\\\.\\pipe\\internal_server'

Window.first_data = true;
Window.Status = "Waiting for connection";

var newServer = function() {
  return net.createServer(function(socket) {
  
  console.log('internal_server connection received');
  sendToServer('connection-established');
  socket.on('data', function(data) {
    if(Window.first_data === true) {
      Window.collecting = true;
      Window.first_data = false;
      Window.Status ="Collecting data";
    }
    var start = performance.now();
    if(Window.visualization_enabled) {
       Window.decoder.unpackStream(data);      
    }
  })

  socket.on('error', function(err) {
    console.log(err);
    Window.collecting = false;
  });

  socket.on('close', function(err) {
    if(!Window.decoder.streamEnd()) {
      console.log("Stream ended incorrectly")
      Window.Status = "Connection ended incorrectly"
    }
    else {
      console.log("The stream Ended correctly")
      Window.collecting = false;
      Window.Status = "Connection ended correctly"
    }
  })
})
}
var server = newServer().listen(list);

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})