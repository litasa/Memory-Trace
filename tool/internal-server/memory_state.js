const Allocator = require('./memory_objects.js')
require('../util/hex_handler.js')
const SortedMap = require("collections/sorted-map");

var MemoryState = function() {
  this.heaps_ = new SortedMap();
  this.current_time_ = Buffer.alloc(8);
}

MemoryState.prototype.createHeap = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = new Heap(data.id, data.nameId);
  this.heaps_.set(data.id, alloc);
  //console.log('MemoryState: created Heap with key: ' + data.id)
  //this.print();
}

MemoryState.prototype.removeHeap = function(data) {
  this.heaps_.delete(data.id);
  //console.log('MemoryState: removed Heap with key: ' + data.id)
  //this.print();
}

MemoryState.prototype.createCore = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.addCore(data);
  //console.log('MemoryState: created Core with key: ' + data.pointer.toString('hex'))
  //this.print();
}

MemoryState.prototype.removeCore = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.removeCore(data);
  //console.log('MemoryState: removed Core with key: ' + data.pointer.toString('hex'));
  //this.print();
}

MemoryState.prototype.createAllocation = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.addAllocation(data);
  //console.log('MemoryState: created Allocation with key: ' + data.pointer.toString('hex'))
  //this.print();
}

MemoryState.prototype.removeAllocation = function(data) {
  this.current_time_ = data.timestamp;
  var alloc = this.heaps_.get(data.id);
  alloc.removeAllocation(data);
  //console.log('MemoryState: removed Allocation with key: ' + data.pointer.toString('hex'))
  //this.print();
}

MemoryState.prototype.print = function() {
  this.heaps_.forEach(function(heap) {
    heap.printMemoryStats();
  })
  console.log('last Update: ' + this.current_time_.readUInt32BE() + ' ' + this.current_time_.readUInt32BE(4))
}

module.exports = MemoryState;