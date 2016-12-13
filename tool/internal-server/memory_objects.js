const SortedMap = require("collections/sorted-map");
const Numeral = require('numeral');

var Heap = function(id, name) {
  this.id_ = id;
  this.name_ = name;
  this.cores_ = new SortedMap();
  this.allocations_ = new SortedMap();
  this.managedMemory_ = Numeral(0);
  this.currentMemory_ = Numeral(0);
};

Heap.prototype.addCore = function(data) {
  this.cores_.set(data.pointer, data);
  this.managedMemory_.add(data.size);
}

Heap.prototype.removeCore = function(data) {
  var core = this.cores_.get(data.pointer);
  this.managedMemory_.subtract(core.size);
  if(!this.cores_.delete(data.pointer)) {
    throw "core does not exist for deletion";
  }
}

Heap.prototype.addAllocation = function(data) {
  this.allocations_.set(data.pointer,data);
  this.currentMemory_.add(data.size);
}

Heap.prototype.removeAllocation = function(data) {
  var alloc = this.allocations_.get(data.pointer);
  this.currentMemory_.subtract(alloc.size);
  if(!this.allocations_.delete(data.pointer)) {
    throw "allocation does not exist for deletion";
  }
}

Heap.prototype.printMemoryStats = function() {
  console.log("Id: " + this.id_ + " Managed memory: " + this.managedMemory_.format('0.00b') + ", Used memory: " + this.currentMemory_.format('0.00b'));
}

module.exports = Heap;
