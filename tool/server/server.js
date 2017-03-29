const net = require('net');
const fs = require('fs');
const stream = require('stream');
const ipcRenderer = require('electron').ipcRenderer
const sendTo = require('../util/sendTo.js')
const moment = require('moment');

var internal_socket;
// external_server
var sockets = [];
var pipe = true;

var external_server = net.createServer(function(socket) {
  sockets.push(socket);
  sendTo.Chart('connection-established')
  console.log("connectin established")
  //var unmodified_stream = fs.createWriteStream("./previous_traces/test_" + time + ".db");
  var unmodified_stream = fs.createWriteStream("./previous_traces/" + moment().format('YYYY-MM-DD_HH-mm-ss') + ".mtrace");

    //write input data to file
    if(pipe){
      socket.pipe(internal_socket);
    }
    socket.pipe(unmodified_stream);

    socket.on('close', function(data) {
      sendTo.Chart('connection-closed')
      console.log("closed connection")
    })
    
    socket.on('connection', function(data) {
      
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

ipcRenderer.on('disable-pipe', function(event, data) {
  if(sockets.length > 0) {
    sockets[0].unpipe(internal_socket)
  }
  pipe=false;
  console.log("unpiped");
})

ipcRenderer.on('enable-pipe', function(event,data) {
  if(sockets.length > 0) {
    sockets[0].pipe(internal_socket)
  }
  pipe=true;
  console.log("piped");
})