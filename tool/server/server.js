const net = require('net');
const fs = require('fs');
const stream = require('stream');
const ipcRenderer = require('electron').ipcRenderer
const sendTo = require('../util/sendTo.js')

var internal_socket;
// external_server
var sockets = [];

var external_server = net.createServer(function(socket) {
  sockets.push(socket);

  var date = new Date();
  var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
  var unmodified_stream = fs.createWriteStream("./previous_traces/test_" + time + ".db");

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
      //needs the event to collect data
    })

    socket.on('error', function(error) {
      var msg = error.name + " " + error.message;
      data = { msg: msg}
      sendTo.Chart('external-error', data);
    })

}).listen(8181);

external_server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendTo.Chart('server-init', {address: add, port: external_server.address().port})
  });
})

external_server.on('error', function(error) {
  var msg = error.name + " " + error.message;
  data = { msg: msg}
  sendTo.Chart('external-error', data);
})

ipcRenderer.on('internal-server-address', function(event, data) {
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