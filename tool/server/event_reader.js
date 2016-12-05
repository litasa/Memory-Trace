var EventReader = new function() {


  global.SeenStacks = new Map();
  global.SeenStrings = new Map();

  global.HeapsAlive = new Map();
  global.HeapsDead = new Map();

  this.oneEvent = function oneStepRead(buffer, fromIndex){
		var data = {};
    var header = {
      event: null,
      scope: null,
	    scopeDataIndex: -1,
      timestamp: null,
      callstackId: null
    };

	  buffer.setRollback();

		if(!readHeaderData(buffer, header)) { return null; }

    if (header.event == global.typeEnum.BeginStream) {
      //TODO -- check that magic number is correct etc
      if(!readBeginStream(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.ModuleDump) {
        if(!readModuleDump(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.HeapCreate) {
      if(!readHeapCreate(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.HeapDestroy) {
      if(!readHeapDestroy(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.HeapAddCore) {
      if(!readHeapAddCore(buffer,data)) { return null; }
    }
		else if(header.event == global.typeEnum.HeapRemoveCore) {
			if(!readHeapRemoveCore(buffer,data)) { return null; }
		}
    else if (header.event == global.typeEnum.HeapAllocate) {
      if(!readHeapAllocate(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.HeapFree) {
      if(!readHeapFree(buffer,data)) { return null; }
    }
    else if (header.event == global.typeEnum.EndStream) {
      //TODO -- Check so that no leaks have happened
    }
		else {
			return null;
		}

		data.header = header

		buffer.setRollback();

		return data;
  }

	function readHeaderData(buffer, data){
		var ret = [];
    if(!STREAM.readByte(buffer, ret)) { return false; }  //code

		if(ret[0] == 0) {
			console.log("unexpected header code: " + ret[0])
		}

    if(!STREAM.readByte(buffer, ret))  { return false; } //scope

    if (ret[1] != 0) {
      if(!STREAM.readStringIndex(buffer, ret)) { return false; } //scope name
	  	data.scopeDataIndex = ret.pop();
    }

    if(!STREAM.read64Byte(buffer, ret)) {return false; } //timestamp
	  if(!STREAM.readBacktraceIndex(buffer, ret)) {return false;} //callstack index

	  data.event       = ret[0];
	  data.scope       = ret[1];
	  data.timestamp   = ret[2];
	  data.callstackId = ret[3];
		//no need for data.type in header
    return true;
  }

  function readBeginStream(buffer, data){
      //TODO -- check that magic number is correct etc
	var ret = [];

	if(!STREAM.read64Byte(buffer, ret)) { return false;} //Stream Magic
	if(!STREAM.readString(buffer, ret)) { return false;} //Platform name
	if(!STREAM.readByte  (buffer, ret)) { return false;} //Pointer size
	if(!STREAM.read64Byte(buffer, ret)) { return false;} //Timer frequency
	if(!STREAM.read64Byte(buffer, ret)) { return false;} //Common address

	data.magicNumber       = ret[0];
	data.platform          = ret[1];
	data.pointerSize       = ret[2];
	data.timerFrequency    = ret[3];
	data.initCommonAddress = ret[4];

	return true;
  }

  function readModuleDump(buffer, data) {
    //TODO -- save symData somewhere
		var tempSymData = [];
	  while(true) {
			var dump = {};
			var ret = [];
			if(!STREAM.readByte(buffer, ret)) {return false;} //keep going code ( 0 == stop )
			if(ret.pop() == 0) {
				break;
			}

			if(!STREAM.readString(buffer, ret)) { return false; } //module name
			if(!STREAM.read64Byte(buffer, ret)) { return false; } //module handle
			if(!STREAM.read64Byte(buffer, ret)) { return false; } //size
			dump.name = ret[0];
			dump.base = ret[1];
			dump.size = ret[2];
			//add to list
			tempSymData.push(dump);
		}
	//add to global modules
		data.symData = tempSymData;

		return true;
  }

  function readHeapCreate(buffer, data) {
	  var Allocator = {};
	  var ret = [];
	  if(!STREAM.readByte(buffer, ret)) { return false; } //id
	  if(!STREAM.readStringIndex(buffer, ret)) { return false; } //name
	  //begin out event
	  data.id     = ret[0];
	  data.nameId = ret[1];

	  return true;
  }

  function readHeapDestroy(buffer, data) {
    var heapDestroy = {};
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) {return false;} //id

		data.id   = ret[0];

		return true;
  }

  function readHeapAddCore(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //pointer to core start
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //size

		data.id    = ret[0];
		data.start = ret[1];
		data.size  = ret[2];

		return true;
  }

	function readHeapRemoveCore(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //pointer to core start
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //size

		data.id    = ret[0];
		data.start = ret[1];
		data.size  = ret[2];

		return true;
	}

  function readHeapAllocate(buffer, data) {
		var ret = [];

		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //pointer to start
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //size

		//TODO check so allocation is not already made
		data.id      = ret[0];
		data.pointer = ret[1];
		data.size    = ret[2];

		return true;
  }

  function readHeapFree(buffer, data) {
		var ret = [];

		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.read64Byte(buffer, ret)) { return false; } //pointer;

		data.id      = ret[0];
		data.pointer = ret[1];

		return true;
  }
} //Eventreader end
