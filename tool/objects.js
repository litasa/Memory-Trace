function Heap(name, timestamp, callstack)
{
  //General heap data
  this.name = name;
  this.birth = timestamp;
  this.death = -1;
  this.callstack = callstack;

  this.cores = new Array();

  this.Allocate = function(allocation){
    var coreid = this.GetCoreForAllocation(allocation);
    if (coreid >= 0) {
      this.cores[coreid].Allocate(allocation);
    }
    else {
      console.log("Could not add Allocation because no core owner found. Not logged");
    }
  }

  this.Deallocate = function(allocation) {
    var coreid = this.GetCoreForAllocation(allocation);
    if (coreid >= 0) {
      this.cores[coreid].Deallocate(allocation);
    }
    else {
      console.log("No core found for deallocation. Not Logged")
    }
  }

  this.DeallocateFromPointer = function(pointer) {
    for (var i = 0; i < this.cores.length; i++) {
      var core = this.cores[i];
      var pos = core.GetPointerPosInAllocationArray(pointer);
      if (pos >= 0) {
        core.Deallocate(core.allocations[pos]);
        return;
      }
    }
    console.log("No pointer found in any core for deallocation. Deallocation not done");
  }

  //Helperfunctions
  this.GetCoreForAllocation = function (allocation){
    for (var i = 0; i < this.cores.length; i++) {
      var core = this.cores[i];


      if (pointerBlockInside(core.start_address,
                            core.end_address,
                            allocation.pointer, allocation.size)){
        return i;
      }
    }
    //core not found
    return -1;
  }

  this.AddCore = function(core) {
    //TODO add checks so cores does not overlap
    this.cores.push(core);
  }

  this.Destroy = function() {
    for (var i = 0; i < this.cores.length; i++) {
      var destroyData = this.cores[i].Destroy();
      if (!destroyData.destroyed) {
        destroyData.name = this.name;
        destroyData.core = i;
        return destroyData;
      }
      else {
        this.cores.splice(i,1);
      }
    }
    return destroyData;
  }

  this.getMemoryUsage = function() {
    var ret = 0;
    for (var i = 0; i < this.cores.length; i++) {
      ret += this.cores[i].getMemoryUsage();
    }
    return ret;
  }
} //end HEAP

function Allocation(pointer, size, creation_time, callstack)
{
  this.pointer = pointer;
  this.size = size;
  this.birth = creation_time;
  this.death = -1;
  this.callstack = callstack;
}

function Core(start_address, size, creation_time, callstackId)
{
  this.start_address = start_address;
  this.end_address = addSizeToPointer(start_address, size);
  this.size = size;
  this.birth = creation_time;
  this.death = -1;
  this.callstackId = callstackId;
  this.allocations = new Array();

  this.pointer = function () { return this.start_address;}

  this.Allocate = function (allocation) {
    var endPoint = addSizeToPointer(allocation.pointer,allocation.size);

    if (pointerLE(endPoint,this.end_address)) {
      this.allocations.push(allocation);
    }
    else {
      console.log("Allocation size too large for Core or Larger then 32 bit. Not logged");
    }
  }

  this.Deallocate = function (allocation) {
    var pos = this.GetPointerPosInAllocationArray(allocation.pointer);
    if (pos >= 0) {
      this.allocations.splice(pos,1);
    }
    else {
      console.log("Pointer position not valid in Core. Allocation not removed");
    }
  }

  this.GetPointerPosInAllocationArray = function(pointer) {
    for (var i = 0; i < this.allocations.length; i++) {
      if (pointerEquals(this.allocations[i].pointer, pointer)) {
        return i;
      }
    }
    return -1;
  }

  this.Destroy = function() {
    if (this.allocations.length > 0) {
      return {
        destroyed: false,
        allocList: this.allocations};
    }
    else {
      return {
        destroyed: true,
      alloclist: null}
    }
  }

  this.getMemoryUsage = function() {
    var ret = 0;
    for (var i = 0; i < this.allocations.length; i++) {
      ret += this.allocations[i].size;
    }
    return ret;
  }
} //end of Core
