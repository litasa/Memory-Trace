const ipcRenderer = require('electron').ipcRenderer
const fs = require('fs');
const RingBuffer = require("./ringbuffer.js")
//const cp = require('child_process')
//const n = cp.fork(`${__dirname}/database.js`)

total_data_handled = 0;
numEvents = 0;
lastTime = 0;
// server
var server = require('net').createServer(function (socket) {
    var last_event;
    var total_data_recieved = 0;
    var first_connect = true;
    var start_time = 0;
    var last_time = 0;
	var ringBuffersize = 128*1024;
	var ringBuffer =  new RingBuffer(ringBuffersize);

  var date = new Date();
  var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
  var name =  "./database/test_" + time + ".db";
    var stream = fs.createWriteStream(name);

    //write input data to file
    socket.pipe(stream);

    socket.on('close', function(data) {
      console.log("socket close")
    })

    socket.on('connect', function(data) {
      console.log("socket connected")
    })

    socket.on('data', function (data) {

      var start = Date.now();
      var daff = performance.now() - last_time;
      console.log("data recieved, time since last: " + daff);
      if(data === undefined) {
        throw "undefined data recieved"
      }

      if (first_connect) {
        first_connect = false;
        start_time = Date.now();
        sendEvent('connection-established', {});
      }

        var buffer = {
          data: Buffer.from(data,'hex'),
          index: 0,
          rollbackNeeded: false,
          rollback: 0
        };

        total_data_handled += data.length;
        var processed = { events: []};

    		do{
    			index = ringBuffer.populate(buffer.data);
    			do{
            var oneEvent = EventReader.oneEvent(ringBuffer);
    				if(oneEvent === null) {
    					break;
    				}
    				else{
    					numEvents++;
              processed.events.push(oneEvent);
    				}
    			} while(ringBuffer.remaining());
          ringBuffer.rollback();
    		} while(index);
        var diff = Date.now() - start;
        console.log("data ended with process time: " + diff);
        last_time = performance.now();
        sendEvent('event-done', {});
    })

    socket.on('drain', function(error) {
      console.log("socket drain")
    })

    socket.on('end', function(data) {
      var diff = Date.now() - start_time;
      console.log("connection ended in: " + diff);
    })

    socket.on('error', function(error) {
      console.log("socket error");
    })

    socket.on('lookup', function(error) {
      console.log("socket lookup")
    })

    socket.on('timeout', function(error) {
      console.log("socket timeout")
    })

})
.listen(8181);

server.on('close', function() {
  console.log("server close");
});

server.on('connection', function(socket) {
  console.log("server connection" + socket);
});

server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendEvent('server-init', {address: add, port: server.address().port})
  });
})

server.on('error', function(error) {
  console.log("server error")
})

sendEvent = function(channel, data) {
  data.channel = channel;
  ipcRenderer.send('to-chart',data);
}
