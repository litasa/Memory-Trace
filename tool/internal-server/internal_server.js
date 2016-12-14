const net = require('net');
const RingBuffer = require("../util/ringbuffer.js");

var ringBuffersize = 128*1024;
var ringBuffer =  new RingBuffer(ringBuffersize);
var numEvents = 0;

var last_time;

var list = '\\\\.\\pipe\\internal_server'

var server = net.createServer(function(socket) {
  console.log('internal_server connection recieved');
  sendToServer('connection-established');

  socket.on('data', function(data) {
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
          numEvents++;
          last_time = event.timestamp;
        }
      } while(ringBuffer.remaining());
      ringBuffer.rollback();
    } while(num_data_left);

   console.log('last logged time: ' + last_time + ' seconds');
  })

  socket.on('error', function(err) {
    console.log(err);
  });

}).listen(list);

server.on('listening', function(data) {
  console.log("listening to: " + server.address())
  sendToServer('please-connect', {addr: list});
})

ipcRenderer.on('stream-end', function(event, data) {
  console.log('finished recieving data: ')
})
