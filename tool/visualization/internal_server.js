const net = require('net');
var enc = require('..\\build\\Release\\Encryption');

var decoder = enc.Decoder();
var event_count = 0;

var last_time = performance.now();
var total_data = 0;

var list = '\\\\.\\pipe\\internal_server'

var update_frequency = 800;

var server = net.createServer(function(socket) {
  
  console.log('internal_server connection recieved');
  sendToServer('connection-established');

  socket.on('data', function(data) {
    var time = performance.now();
    var diff = time - last_time;
    console.log("time since last on data event: " + diff );
    total_data += data.length;
    console.log("start: \n" + "Total data recieved: " + total_data);
    var start = performance.now();
      decoder.unpackStream(data);
      var arr = decoder.getMemoryAsArray();
      Visualization.newDataset(arr);
      console.log(decoder.getCurrentMemory());
      
    var end = performance.now();
    console.log("unpackStream and getMemoryAsArray took" + (end - start));
    last_time = performance.now();
  })

  socket.on('error', function(err) {
    console.log(err);
  });

}).listen(list);

server.on('listening', function(data) {
  console.log("listening to: " + server.address())
})

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})