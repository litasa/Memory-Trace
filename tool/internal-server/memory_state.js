const Allocator = require('./memory_objects.js')

//var list = 8182;
//var list = '\\\\.\\pipe\\internal_server'

var MemoryState = function() {
  this.allocators_ = new Map();

  this.cores_ = new Map();

  this.managedMemory = 0;
  
  this.usedMemory = 0;
}

MemoryState.prototype.createAllocator = function(data) {
  var alloc = new Allocator(data.id, data.nameId);
  this.allocators_.set(data.id, alloc);
  console.log('Allocator added')
}

MemoryState.prototype.createCore = function(data) {
  var allocator = this.allocators_.get(data.id);
  allocator.addCore(data);
  console.log('Core added')
}

MemoryState.prototype.createAllocation = function(data) {
  var allocator = this.allocators_.get(data.id);
  allocator.addAllocation(data.pointer);
  console.log('Allocation added')
}

MemoryState.prototype.removeAllocator = function(data) {
  var allocator = this.allocators_.get(data.id);
  var coresToRemove = allocator.getCores();
  var allocationsToRemove = allocator.getAllocations();
  coresToRemove.forEach(function(element) {
    this.cores_.delete(element)
    console.log("this should not happen")
  }, this)
  allocationsToRemove.forEach(function(element) {
    this.allocations_.delete(element)
    console.log("this should not happen")
  }, this)
  this.allocators_.delete(data.id);
}

MemoryState.prototype.removeCore = function(data) {
  var allocator = this.allocators_.get(data.id);

}

module.exports = MemoryState;