const ipcRenderer = require('electron').ipcRenderer


connected = false;

window.onload = function() {
  initChart();
  }

ipcRenderer.once('server-init', function(event, serverData) {
  server = {};
  server.address = serverData.address;
  server.port = serverData.port;

  console.log('address: ' + server.address + ' port: ' + server.port);
})

ipcRenderer.once('connection-established', function() {
  connected = true;
})

ipcRenderer.on('event-done', function(event, data) {
  console.log('data recieved: ')
  /*
  for (var i = 0; i < data.events.length; i++) {
    var event = data.events[i];
    var eventID = event.header.event;

    if (eventID == global.typeEnum.BeginStream) {
      Visualization.beginStream(event);
    }
    else if (eventID == global.typeEnum.ModuleDump) {

    }
    else if (eventID == global.typeEnum.HeapCreate) {
      Visualization.addAllocator(event);
    }
    else if (eventID == global.typeEnum.HeapDestroy) {

    }
    else if (eventID == global.typeEnum.HeapAddCore) {
      Visualization.addCore(event);
    }
    else if(eventID == global.typeEnum.HeapRemoveCore) {

    }
    else if (eventID == global.typeEnum.HeapAllocate) {
      Visualization.addAllocation(event);
    }
    else if (eventID == global.typeEnum.HeapFree) {
      Visualization.removeAllocation(event);
    }
    else if (eventID == global.typeEnum.EndStream) {
      //TODO -- Check so that no leaks have happened
    }
    else {
      return null;
    }
  }
  //Visualization.update();
  */
})
