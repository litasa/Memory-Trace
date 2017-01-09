const net = require('net');
var enc = require('..\\build\\Release\\Encryption');
const RingBuffer = require("../util/ringbuffer.js");

var decoder = enc.Decoder();
var ringBuffersize = 128*1024;
var ringBuffer =  new RingBuffer(ringBuffersize);
var event_count = 0;

var last_time;
var total_data = 0;

var list = '\\\\.\\pipe\\internal_server'

var server = net.createServer(function(socket) {
  
  console.log('internal_server connection recieved');
  sendToServer('connection-established');

  socket.on('data', function(data) {
    total_data += data.length;
    console.log("start: \n" + "Total data recieved: " + total_data);
    decoder.unpackStream(data);

    var arr = decoder.getMemoryAsArray();
    for(var i = 0; i < arr.length; ++i) {
      console.log(arr[i].name + " " + arr[i].used_memory);
    }
    // var arr = decoder.getMemoryAsArray();
    // //var barr = decoder.getNewEvents();
    // sendToChart("memory", {array: arr});
    // console.log("printing heaps")
    // for(var i = 0; i < arr.length; ++i) {
    //   console.log(arr[i].name + " : " + arr[i].used_memory + " last update: " + arr[i].last_update);
    // }
    /*
    var num_data_left = data.length;
    //console.log('data recieved internal_server: ' + data.length )
    do{
      //add as much as possible to the ringBuffer
      num_data_left = ringBuffer.populate(data, num_data_left);
      do{
        //read one event
        var event = EventDecoder.getEvent(ringBuffer);
        if(event === null) {
          // break if unsucessful
          break;
        }
        else{
          event_count++;
          last_time = event.timestamp;
        }
      } while(ringBuffer.remaining());
      ringBuffer.rollback();
    } while(num_data_left);

   console.log('Registerd events: ' + event_count)
    */
   console.log("done");
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

window.onload = function() {
  setTimeout(function() {
    sendToServer('please-connect', {addr: list});
  },2000);
  
}
