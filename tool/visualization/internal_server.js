const net = require('net');
var event_count = 0;

var last_time = performance.now();
var total_data = 0;

var list = '\\\\.\\pipe\\internal_server'

var update_frequency = 800;
Window.first_data = true;
var server = newServer().listen(list);

var newServer = function() {
  return net.createServer(function(socket) {
  
  console.log('internal_server connection received');
  sendToServer('connection-established');
  socket.on('data', function(data) {
    if(Window.first_data === true) {
      Window.collecting = true;
      Window.first_data = false;
    }
    total_data += data.length;
    var start = performance.now();
    if(Window.visualization_enabled) {
       Window.decoder.unpackStream(data);      
    }
    if(!Window.decoder.streamEnd()) {
      
    }
    else {
      console.log("The stream Ended correctly")
      Window.collecting = false;
    }
  })

  socket.on('error', function(err) {
    console.log(err);
    Window.collecting = false;
  });

})
}

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})