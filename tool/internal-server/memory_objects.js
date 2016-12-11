require('hex_handler.js')

var Allocator = function(id, name) {
  this.id_ = id;
  this.name_ = name;
  this.cores_ = new Map();
};

Allocator.prototype.addCore = function (core){
  this.cores_.set(core.pointer,core);
}

Allocator.prototype.addAllocation = function (allocation) {
  this.allocations_.set(allocation.pointer,allocation);
}



var Core = function(pointer, size) {
  this.pointer_ = pointer;
  this.size_ = size;
}

module.exports = Allocator;
