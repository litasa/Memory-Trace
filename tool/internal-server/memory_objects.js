const SortedMap = require("collections/sorted-map");

var Heap = function(id, name) {
  this.id_ = id;
  this.name_ = name;
  this.cores_ = new SortedMap();
  this.allocations_ = new SortedMap();
  this.managedMemory_ = 0;
  this.currentMemory_ = 0;
};

Heap.prototype.addCore = function(data) {
  this.cores_.set(data.pointer, data);
  this.managedMemory_ += data.size.readUInt32LE();
}

Heap.prototype.removeCore = function(data) {
  var core = this.cores_.get(data.pointer);
  this.managedMemory_ -= core.size.readUInt32LE();
  if(!this.cores_.delete(data.pointer)) {
    throw "core does not exist for deletion";
  }
}

Heap.prototype.addAllocation = function(data) {
  this.allocations_.set(data.pointer,data);
  this.currentMemory_ += data.size.readUInt32LE();
}

Heap.prototype.removeAllocation = function(data) {
  var alloc = this.allocations_.get(data.pointer);
  this.currentMemory_ -= alloc.size.readUInt32LE();
  if(!this.allocations_.delete(data.pointer)) {
    throw "allocation does not exist for deletion";
  }
}

Heap.prototype.printMemoryStats = function() {
  console.log("Id: " + this.id_ + " Managed memory: " + this.managedMemory_ + ", Used memory: " + this.currentMemory_);
}

module.exports = Heap;
