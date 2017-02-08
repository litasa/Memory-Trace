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
    total_data += data.length;
    var start = performance.now();
      decoder.unpackStream(data);
      var arr = decoder.getMemoryAsArray();
      var removed_heaps = decoder.getDeadHeaps();
      arr = _.compact(arr);
      Visualization.newDataset(arr);
      Visualization.removeDatasets(removed_heaps);
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