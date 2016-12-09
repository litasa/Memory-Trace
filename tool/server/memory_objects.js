var Allocator = function(id, name) {
  this.id_ = id;
  this.name_ = name;
  this.cores_ = new Array();
  this.allocations_ = new Array();
};

Allocator.prototype.addCore = function (data){
  this.cores_.push({pointer_: data.pointer, size_: data.size});
}

Allocator.prototype.addAllocation = function (data) {
  this.allocations_.push({pointer_: data.pointer, size_: data.size});
}

module.exports = Allocator;
