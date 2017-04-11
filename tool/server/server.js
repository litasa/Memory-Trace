const net = require('net');
const fs = require('fs');
const stream = require('stream');
const ipcRenderer = require('electron').ipcRenderer
<<<<<<< HEAD
const send_to = require('../util/send_to.js')
=======
const sendTo = require('../util/sendTo.js')
>>>>>>> event_struct_system
const moment = require('moment');

var internal_socket;
// external_server
var sockets = [];
var pipe = true;

var external_server = net.createServer(function(socket) {
  sockets.push(socket);
<<<<<<< HEAD
  send_to.Chart('connection-established')
=======
  sendTo.Chart('connection-established')
>>>>>>> event_struct_system
  console.log("connectin established")
  //var unmodified_stream = fs.createWriteStream("./previous_traces/test_" + time + ".db");
  var unmodified_stream = fs.createWriteStream("./previous_traces/" + moment().format('YYYY-MM-DD_HH-mm-ss') + ".mtrace");

    //write input data to file
    if(pipe){
      socket.pipe(internal_socket);
    }
    socket.pipe(unmodified_stream);

    socket.on('close', function(data) {
<<<<<<< HEAD
      send_to.Chart('connection-closed')
=======
      sendTo.Chart('connection-closed')
>>>>>>> event_struct_system
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
<<<<<<< HEAD
      send_to.Chart('external-error', data);
=======
      sendTo.Chart('external-error', data);
>>>>>>> event_struct_system
    })

}).listen(8181);

external_server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
<<<<<<< HEAD
    send_to.Chart('server-init', {address: add, port: external_server.address().port})
=======
    sendTo.Chart('server-init', {address: add, port: external_server.address().port})
>>>>>>> event_struct_system
  });
})

external_server.on('error', function(error) {
  var msg = error.name + " " + error.message;
  data = { msg: msg}
<<<<<<< HEAD
  send_to.Chart('external-error', data);
=======
  sendTo.Chart('external-error', data);
>>>>>>> event_struct_system
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