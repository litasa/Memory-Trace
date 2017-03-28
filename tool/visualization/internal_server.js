const net = require('net');
const status = require("./status.js");

var list = '\\\\.\\pipe\\internal_server'

var newServer = function() {
  return net.createServer(function(socket) {
  var first_data = true;
  sendToServer('connection-established');

  socket.on('data', function(data) {
    
    var string = "Collecting data"
    Window.decoder.unpackStream(data);
    status.collecting = true;    
  })

  socket.on('error', function(err) {
    console.log(err);
    status.collecting = false;
  });

  socket.on('close', function(err) {
    if(!Window.decoder.streamEnd()) {
      console.log("Stream ended incorrectly")
      status.SetMessage("Connection ended incorrectly");
    }
    else {
      console.log("The stream Ended correctly")
      status.collecting = false;
      status.SetMessage("Connection ended correctly");
    }
  })
})
}
var server = newServer().listen(list);

ipcRenderer.on('stream-start', function(event, data) {
  status.SetMessage("Collecting data");
})

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})