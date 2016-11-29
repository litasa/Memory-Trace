var EventReader = new function() {
	global.typeEnum = {
    'BeginStream' : 1,
    'EndStream': 2,
    'ModuleDump': 3,
    'Mark': 4,

    'AddressAllocate': 10,
    'AddressFree': 11,
    'VirtualCommit': 12,
    'VirtualDecommit': 13,

    'PhysicalAllocate': 14,
    'PhysicalFree': 15,
    'PhysicalMap': 16,
    'PhysicalUnmap': 17,

    'HeapCreate': 18,
    'HeapDestroy': 19,
    'HeapAddCore': 20,
    'HeapRemoveCore': 21,
    'HeapAllocate': 22,
    'HeapReallocate': 23,
    'HeapFree': 24
  };

  global.SeenStacks = new Map();
  global.SeenStrings = new Map();

  global.HeapsAlive = new Map();
  global.HeapsDead = new Map();

  this.oneEvent = function oneStepRead(buffer, fromIndex){
    var header = {
      event: null,
      scope: null,
	  scopeDataIndex: -1,
      timestamp: null,
      callstackId: null
    };
	buffer.rollback = buffer.read;
    if(!readHeaderData(header, buffer)) { return false; }

    if (header.event == global.typeEnum.BeginStream) {
      //TODO -- check that magic number is correct etc
      if(!readBeginStream(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.ModuleDump) {
        if(!readModuleDump(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.HeapCreate) {
      if(!readHeapCreate(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.HeapDestroy) {
      if(!readHeapDestroy(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.HeapAddCore) {
      if(!readHeapAddCore(buffer,header)) { return false; }
    }
		else if(header.event == global.typeEnum.HeapRemoveCore) {
			if(!readHeapRemoveCore(buffer,header)) { return false; }
		}
    else if (header.event == global.typeEnum.HeapAllocate) {
      if(!readHeapAllocate(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.HeapFree) {
      if(!readHeapFree(buffer,header)) { return false; }
    }
    else if (header.event == global.typeEnum.EndStream) {
      //TODO -- Check so that no leaks have happened
    }
		else {
			return false;
		}
    //var totalMemory = getTotalMemory(global.HeapsAlive);
    //addChartData(meep, { x: header.timestamp, y: totalMemory});
	return true;
  }

	function readHeaderData(header, buffer){
	var ret = [];
    if(!STREAM.readByte(ret, buffer)) { return false; }  //code
    if(!STREAM.readByte(ret, buffer))  { return false; } //scope

    if (ret[1] != 0) {
      if(!STREAM.readStringIndex(ret, buffer)) { return false; } //scope name
	  	header.scopeDataIndex = ret.pop();
    }

    if(!STREAM.read64Byte(ret, buffer)) {return false; } //timestamp
	if(!STREAM.readBacktraceIndex(ret, buffer)) {return false;} //callstack index

	header.event       = ret[0];
	header.scope       = ret[1];
	header.timestamp   = ret[2];
	header.callstackId = ret[3];

    return true;
  }

  function readBeginStream(buffer, head){
      //TODO -- check that magic number is correct etc
	var beginStreamEvent = {};
	var ret = [];

	if(!STREAM.read64Byte(ret, buffer)) { return false;} //Stream Magic
	if(!STREAM.readString(ret, buffer)) { return false;} //Platform name
	if(!STREAM.readByte(ret, buffer))   { return false;} //Pointer size
	if(!STREAM.read64Byte(ret, buffer)) { return false;} //Timer frequency
	if(!STREAM.read64Byte(ret, buffer)) { return false;} //Common address

	beginStreamEvent.magicNumber       = ret[0];
	beginStreamEvent.platform          = ret[1];
	beginStreamEvent.pointerSize       = ret[2];
	global.timerFrequency   		   = ret[3];
	beginStreamEvent.initCommonAddress = ret[4];

	return true;
  }

  function readModuleDump(buffer, head) {
    //TODO -- save symData somewhere
	var tempSymData = [];
    while(true)
	{
		var dump = {};
		var ret = [];
		if(!STREAM.readByte(ret,buffer)) {return false;} //keep going code ( 0 == stop )
		if(ret.pop() == 0)
		{
			break;
		}

		if(!STREAM.readString(ret, buffer)) { return false; } //module name
		if(!STREAM.read64Byte(ret, buffer)) { return false; } //module handle
		if(!STREAM.read64Byte(ret, buffer)) { return false; } //size
		dump.name = ret[0];
		dump.base = ret[1];
		dump.size = ret[2];
		//add to list
		tempSymData.push(dump);
	}
	//add to global modules
	global.Modules = tempSymData;

	return true;
  }

  function readHeapCreate(buffer, head) {
	  var Allocator = {};
	  var ret = [];
	  if(!STREAM.readByte(ret, buffer)) { return false; } //id
	  if(!STREAM.readStringIndex(ret,buffer)) { return false; } //name
	  //begin out event
	  Allocator.head   = head;
	  Allocator.id     = ret[0];
	  Allocator.nameId = ret[1];

	  Visualization.addAllocator(Allocator);

	  return true;
  }

  function readHeapDestroy(buffer, head) {
    var heapDestroy = {};
	var ret = [];
	if(!STREAM.readByte(ret,buffer)) {return false;} //id

	heapDestroy.head = head;
	heapDestroy.id   = ret[0];

	return true;
  }

  function readHeapAddCore(buffer,head) {
    var core = {};
		var ret = [];
		if(!STREAM.readByte(ret,buffer)) { return false; } //id
		if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer to core start
		if(!STREAM.read64Byte(ret,buffer)) { return false; } //size

		core.head  = head;
		core.id    = ret[0];
		core.start = ret[1];
		core.size  = ret[2];

		Visualization.addCore(core);

		return true;
  }

	function readHeapRemoveCore(buffer,head) {
		var ret = [];
		var core = {};
		if(!STREAM.readByte(ret,buffer)) { return false; } //id
		if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer to core start
		if(!STREAM.read64Byte(ret,buffer)) { return false; } //size

		core.head  = head;
		core.id    = ret[0];
		core.start = ret[1];
		core.size  = ret[2];

		//Visualization.removeCore(core);

		return true;
	}

  function readHeapAllocate(buffer, head) {
    var allocation = {};
	var ret = [];

	if(!STREAM.readByte(ret,buffer)) { return false; } //id
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer to start
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //size

	//TODO check so allocation is not already made
	allocation.id      = ret[0];
	allocation.pointer = ret[1];
	allocation.size    = ret[2];
	allocation.head    = head;

	Visualization.addAllocation(allocation);

	return true;
  }

  function readHeapFree(buffer,head) {
    var heapFree = {};
	var ret = [];

	if(!STREAM.readByte(ret,buffer)) { return false; } //id
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer;

	heapFree.head    = head;
	heapFree.id      = ret[0];
	heapFree.pointer = ret[1];

	Visualization.removeAllocation(heapFree);

	return true;
  }
}
function getEventAsString(event)
{
  var string = new String();
  string = "(" + event +")";
  if(event == 1){
    return string.concat("BeginStream");
  }  else if (event == 2) {
    return string.concat("EndStream");
  }  else if (event == 3) {
    return string.concat("ModuleDump");
  }  else if (event == 4) {
    return string.concat("Mark");
  }  else if (event == 10) {
    return string.concat("AddressAllocate");
  } else if (event == 11) {
      return string.concat("AddressFree");
  }  else if (event == 12) {
      return string.concat("VirtualCommit");
  }  else if (event == 13) {
      return string.concat("VirtualDecommit");
  }  else if (event == 14) {
      return string.concat("PhysicalAllocate");
  }  else if (event == 15) {
      return string.concat("PhysicalFree");
  }  else if (event == 16) {
      return string.concat("PhysicalMap");
  }  else if (event == 17) {
      return string.concat("PhysicalUnmap");
  }  else if (event == 18) {
      return string.concat("HeapCreate");
  }  else if (event == 19) {
      return string.concat("HeapDestroy");
  }  else if (event == 20) {
      return string.concat("HeapAddCore");
  }  else if (event == 21) {
      return string.concat("HeapRemoveCore");
  }  else if (event == 22) {
      return string.concat("HeapAllocate");
  }  else if (event == 23) {
      return string.concat("HeapReallocate");
  }  else if (event == 24) {
      return string.concat("HeapFree");
  }
  else {
    return string.concat("Unhandled Event");
  }
}
