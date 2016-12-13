const MemoryState = require('./memory_state.js');

var currentState = new MemoryState();

const net = require('net');
const ipcRenderer = require('electron').ipcRenderer
const RingBuffer = require("../util/ringbuffer.js");

var ringBuffersize = 128*1024;
var ringBuffer =  new RingBuffer(ringBuffersize);
var numEvents = 0;

var last_time;

var list = '\\\\.\\pipe\\internal_server'

var interval = setInterval(function() {
  currentState.print();
},1000)

var server = net.createServer(function(socket) {
  console.log('Connected to internal_server');

  socket.on('data', function(data) {
    var num_data_left = data.length;
    //console.log('data recieved internal_server: ' + data.length )
    do{
      //add as much as possible to the ringBuffer
      num_data_left = ringBuffer.populate(data, num_data_left);
      do{
        //read one event
        var oneEvent = EventReader.oneEvent(ringBuffer, currentState);
        if(oneEvent === null) {
          // break if unsucessful
          break;
        }
        else{
          numEvents++;
          last_time = oneEvent.timestamp;
        }
      } while(ringBuffer.remaining());
      ringBuffer.rollback();
    } while(num_data_left);

   console.log('last logged time: ' + last_time.readUInt32BE() + ' ' + last_time.readUInt32BE(4))
  })

  socket.on('error', function(err) {
    console.log(err);
  });

}).listen(list);

server.on('listening', function(data) {
  console.log("listening to: " + server.address())
  sendEvent('please-connect', {addr: list});
})

sendEvent = function(channel, data) {
  if(data === undefined) {
    data = {}
  }
  data.channel = channel;
  ipcRenderer.send('to-server',data);
}

ipcRenderer.on('stream-end', function(event, data) {
  clearInterval(interval);
  console.log('last state of memory');
  currentState.print();
})
