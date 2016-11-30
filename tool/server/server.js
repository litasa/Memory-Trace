const ipc = require('electron').ipcRenderer
const BrowserWindow = require('electron').remote.BrowserWindow
const RingBuffer = require("./ringbuffer.js")
const visualizeWindow = BrowserWindow.fromId(1)
total_data_handled = 0;
numEvents = 0;
lastTime = 0;
// server
var server = require('net').createServer(function (socket) {
    console.log("connected");
    var total_data_recieved = 0;

	var ringBuffersize = 128*1024;
	var ringBuffer =  new RingBuffer(ringBuffersize);

    socket.on('data', function (data) {
        sendEvent('connection-established');
        var buffer = {
          data: Buffer.from(data,'hex'),
          index: 0,
          rollbackNeeded: false,
          rollback: 0
        };

        total_data_handled += data.length;
    		do{
    			index = ringBuffer.populate(buffer.data);
    			do{
    				if(!EventReader.oneEvent(ringBuffer)) {
    					break;
    				}
    				else{
    					numEvents++;
    				}
    			} while(ringBuffer.remaining());
          ringBuffer.rollback();
    		} while(index);
        var sendData = {
          total_handled: total_data_handled,
          total_recieved: total_data_recieved,
          number_of_events: numEvents
        }
        sendEvent('event-done', sendData);
    })

    socket.on('end', function(data) {
      console.log("connection ended")
    })

})
.listen(8080);

server.on('close', function() {
  console.log("connection closed, hmm");
});

server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendEvent('server-init', {address: add, port: server.address().port})
  });
})

ipc.on('get-data', function(event) {
    sendEvent('data-sent', JSON.stringify({x: lastTime, y:numEvents}));
})

sendEvent = function(channel, data) {
  visualizeWindow.webContents.send(channel,data);
}
