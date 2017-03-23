const net = require('net');
const fs = require('fs');
const stream = require('stream');

var internal_socket;
var external_socket;
// external_server

var newServer = function() {
  return net.createServer(function (socket) {
  external_socket = socket;
  var total_data_recieved = 0;
  var first_connect = true;
  var start_time = 0;
  var last_time = 0;

  var date = new Date();
  var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
  var name =  "./previous_traces/test_" + time + ".db";
  var unmodified_stream = fs.createWriteStream(name);

    //write input data to file
    socket.pipe(internal_socket);
    socket.pipe(unmodified_stream);

    socket.on('close', function(data) {
      console.log("socket close")
      sendToChart('connection-closed')
      
    })

    socket.on('connect', function(data) {
      console.log("socket connected")
      sentToChart('connection-established')   
    })

    var counter = 0;
    socket.on('data', function (data) {
      if(counter % 500 == 0) {
        counter = 0;
        console.log("data recieved");
      }
      counter++;
      if(data === undefined) {
        throw "undefined data recieved"
      }

      total_data_recieved += data.length;
    })

    socket.on('drain', function(error) {
      console.log("socket drain")
    })

    socket.on('end', function(data) {
      var diff = performance.now() - start_time;
      console.log("connection ended in: " + diff + ", with sent data: " + total_data_recieved);
      sendToChart('connection-closed')
    })

    socket.on('error', function(error) {
      console.log("External socket error: " + error.name + " with message: " + error.message);
    })

    socket.on('lookup', function(error) {
      console.log("socket lookup")
    })

    socket.on('timeout', function(error) {
      console.log("socket timeout")
    })

})
}

var external_server = newServer().listen(8181);

external_server.on('close', function() {
  console.log("server closed at time: " + performance.now());
  sendToChart('connection-closed')
});

external_server.on('connection', function(socket) {
  console.log('connection to Memtrace::InitSocket (or open file) done');
});

external_server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendToChart('server-init', {address: add, port: external_server.address().port})
  });
})

external_server.on('error', function(error) {
  console.log("server error: " + error)
})

ipcRenderer.on('please-connect', function(event, data) {
  internal_socket = net.createConnection(data.addr);
})

ipcRenderer.on('pause', function(event, data) {
  external_socket.write('pause\0');
})

ipcRenderer.on('resume', function(event, data) {
  external_socket.write('resume\0');
})

ipcRenderer.on('callstack', function(event, data) {
  external_socket.write('callstack\0');
})