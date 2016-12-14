const Heap = require('./memory_objects.js')
const SortedMap = require("collections/sorted-map");

var MemoryState = function() {
  this.heaps_ = new SortedMap();
  this.current_time_ = Buffer.alloc(8);
}

MemoryState.prototype.createHeap = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = new Heap(data.id, data.nameId, data.timestamp);
  this.heaps_.set(data.id, alloc);
  sendToChart('heap-created', {id: data.id, timestamp: data.timestamp});
}

MemoryState.prototype.removeHeap = function(data) {
  this.heaps_.delete(data.id);
}

MemoryState.prototype.createCore = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.addCore(data);
}

MemoryState.prototype.removeCore = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.removeCore(data);
}

MemoryState.prototype.createAllocation = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.addAllocation(data);
  sendToChart('new-allocation', {id: data.id, timestamp: data.timestamp, size: alloc.currentMemory_.value()})
}

MemoryState.prototype.removeAllocation = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.removeAllocation(data);
}

MemoryState.prototype.print = function() {
  this.heaps_.forEach(function(heap) {
    heap.printMemoryStats();
  })
  console.log('last Update: ' + this.current_time_.readUInt32BE() + ' ' + this.current_time_.readUInt32BE(4))
}

module.exports = MemoryState;