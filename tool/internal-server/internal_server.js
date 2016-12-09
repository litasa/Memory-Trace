const net = require('net');
const ipcRenderer = require('electron').ipcRenderer
const ReadStream = require('../util/read_stream.js')
const WriteStream = require('../util/write_stream.js')
const RingBuffer = require("../util/ringbuffer.js");


var read_stream = new ReadStream();
var write_stream = new WriteStream();

//read_stream.pipe(write_stream);

var ringBuffersize = 128*1024;
var ringBuffer =  new RingBuffer(ringBuffersize);
var numEvents = 0;
var processed = {events: []};
//var list = 8182;
var list = '\\\\.\\pipe\\internal_server'
var arr = [];
var server = net.createServer(function(socket) {
  console.log('Connected to internal_server');

  socket.on('data', function(data) {
    var num_data_left = data.length;
    console.log('data recieved internal_server: ' + data.length )
    do{
      //add as much as possible to the ringBuffer
      num_data_left = ringBuffer.populate(data, num_data_left);
      do{
        //read one event
        var oneEvent = EventReader.oneEvent(ringBuffer);
        if(oneEvent === null) {
          // break if unsucessful
          break;
        }
        else{
          numEvents++;
          processed.events.push(oneEvent);
        }
      } while(ringBuffer.remaining());
      ringBuffer.rollback();
    } while(num_data_left);
  })

  socket.on('error', function(err) {
    console.log(err);
  });

}).listen(list);

server.on('listening', function(data) {
  console.log("listening to: " + server.address())
})

sendEvent = function(channel, data) {
  if(data === undefined) {
    data = {}
  }
  data.channel = channel;
  ipcRenderer.send('to-server',data);
}

ipcRenderer.on('memory-usage', function(event, data) {
  console.log("recieved memory")
})
