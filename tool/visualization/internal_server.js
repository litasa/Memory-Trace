const net = require('net');
var event_count = 0;

var last_time = performance.now();
var total_data = 0;

var list = '\\\\.\\pipe\\internal_server'

var update_frequency = 800;
first_data = true;
var server = net.createServer(function(socket) {
  
  console.log('internal_server connection recieved');
  sendToServer('connection-established');
  socket.on('data', function(data) {
    if(first_data === true) {
      Window.started = true;
      first_data = false;
    }
    total_data += data.length;
    var start = performance.now();
      Window.decoder.unpackStream(data);
      if(!Window.decoder.streamEnd()) {
        var win_size = parseInt(document.getElementById('window_size').value)
		var min_value = Math.max(Visualization.chart.scales['x-axis-0'].end - win_size, 0);
		var heap_id = 1;
    var max_samples_per_second = 10;
    Visualization.chart.data.datasets = Window.decoder.getFilteredMemorySnapshots(win_size, min_value, heap_id, 10);
        //var heaps = decoder.getAliveHeaps();
        //var arr = Window.decoder.getMemoryAsArray();
        //var removed_heaps = Window.decoder.getDeadHeaps();
        //arr = _.compact(arr);
        //Visualization.newDataset(arr);
        //Visualization.removeDatasets(removed_heaps);
      }
      else {
        console.log("The stream Ended correctly")
        Window.started = false;
      }
  })

  socket.on('error', function(err) {
    console.log(err);
    Window.started = false;
  });

}).listen(list);

server.on('listening', function(data) {
  console.log("listening to: " + server.address())
})

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ' + total_data);
})