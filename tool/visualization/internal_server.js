const net = require('net');
const status = require("./status.js");
var sendTo = require("../util/sendTo.js")

var list = '\\\\.\\pipe\\internal_server'

var server = net.createServer(function(socket) {
  socket.on('data', function(data) {
    Window.decoder.unpackStream(data);
    if(!status.collecting) {
      status.SetMessage("Collecting data")
    }
    status.collecting = true;
  })

  socket.on('error', function(err) {
    console.log(err);
    status.collecting = false;
  });

  socket.on('close', function(err) {
    if(!Window.decoder.streamEnd()) {
      console.log("Stream ended incorrectly")
      status.SetWarningMessage("Connection ended incorrectly");
    }
    else {
      status.collecting = false;
      status.SetMessage("Connection ended correctly");
    }
  })
}).listen(list);