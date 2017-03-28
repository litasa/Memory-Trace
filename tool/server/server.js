const net = require('net');
const fs = require('fs');
const stream = require('stream');
const ipcRenderer = require('electron').ipcRenderer
const sendTo = require('../util/sendTo.js')

var internal_socket;
var external_socket;
// external_server

var newServer = function() {
  return net.createServer(function (socket) {
  external_socket = socket;

  var date = new Date();
  var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
  var name =  "./previous_traces/test_" + time + ".db";
  var unmodified_stream = fs.createWriteStream(name);

    //write input data to file
    socket.pipe(internal_socket);
    socket.pipe(unmodified_stream);

    socket.on('close', function(data) {
      sendTo.Chart('connection-closed')
    })

    socket.on('connect', function(data) {
      sentTo.Chart('connection-established')   
    })

    socket.on('data', function (data) {
      //could add a counter on data.length to get recieved data
    })

    socket.on('end', function(data) {
      sendTo.Chart('connection-closed')
    })
})
}

var external_server = newServer().listen(8181);

external_server.on('close', function() {
  console.log("server closed at time: " + performance.now());
  sendTo.Chart('connection-closed')
});

external_server.on('connection', function(socket) {
  console.log('connection to Memtrace::InitSocket (or open file) done');
});

external_server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendTo.Chart('server-init', {address: add, port: external_server.address().port})
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