const Allocator = require('./memory_objects.js')

//var list = 8182;
//var list = '\\\\.\\pipe\\internal_server'
var allocatorList = new Array();

createAllocator = function(data) {
  var alloc = new Allocator(data.id, data.nameId);
  allocatorList.push(alloc);
  console.log('Allocator added')
}

createCore = function(data) {
  var allocator = getAllocator(data.id);
  allocator.addCore(data);
  console.log('Core added')
}

createAllocation = function(data) {
  var allocator = getAllocator(data.id);
  allocator.addAllocation(data);
  console.log('Allocation added')
}

ipcRenderer.on('request-memory-usage', function(event, data) {
  sendEvent('memory-usage', {memory: 0});
})


getIndex = function (array, id) {
  var index = array.map(function(obj) {return obj.id_; }).indexOf(id);
  if(index == -1) {
    throw "Id: " + id + " does not exist in array: " + array;
  }
  return index;
}

getAllocator = function(id) {
  return allocatorList[getIndex(allocatorList,id)];
}
