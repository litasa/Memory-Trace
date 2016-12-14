
var memory_state = new MemoryState();

var EventDecoder = new function() {
	var time = new TimeConverter();
	this.getEvent = function getEvent(buffer){
		var data = {};
	  	buffer.setRollback();
		
		//Start a new event read, contains eventType and timestamp
		if(!BeginEvent(buffer, data)) { return null; }

		switch(data.event) {
			case(Enum.BeginStream) :
      			if(!BeginStream(buffer,data)) {
					return null;
				}
				//TODO -- check that magic number is correct etc
			break;

			case(Enum.HeapCreate) :
				if(!HeapCreate(buffer,data)) {
					return null;
				}
				memory_state.createHeap(data);
			break;

			case(Enum.HeapDestroy) :
				if(!HeapDestroy(buffer,data)) {
					return null;
				}
			break;

			case(Enum.HeapAddCore) :
				if(!HeapAddCore(buffer,data)) {
					return null;
				}
			break;

			case(Enum.HeapRemoveCore) :
				if(!HeapRemoveCore(buffer,data)) {
					return null;
				}
			break;

			case(Enum.HeapAllocate) :
				if(!HeapAllocate(buffer,data)) {
					return null;
				}
				memory_state.createAllocation(data);
			break;

			case(Enum.HeapFree) :
				if(!HeapFree(buffer,data)) {
					return null;
				}
			break;

			case(Enum.EndStream) :
				//TODO -- Check so that no leaks have happened
				//no additional data sent other than the Begin Event
			break;
			default:
				//Unknown eventcode results in no read
				return null;
		} //end of switch

		//could probably be omitted, but is here for safety.
		buffer.setRollback();

		return data;
	}

	function BeginEvent(buffer, data) {
		var ret = [];
    	if(!STREAM.readByte(buffer, ret)) { return false; } //code
    	if(!STREAM.readByte(buffer, ret)) { return false; } //timestamp

		data.event     = ret[0].readUInt32BE(4);
		data.timestamp = time.getSeconds(ret[1]);
		
		return true;
	}
	
	function BeginStream(buffer, data) {
		var ret = [];

		if(!STREAM.readByte(buffer, ret))   { return false;} //Stream Magic
		if(!STREAM.readString(buffer, ret)) { return false;} //Platform name
		if(!STREAM.readByte(buffer, ret))   { return false;} //Timer frequency

		data.magicNumber    = ret[0];
		data.platform       = ret[1];
		data.timerFrequency = ret[2];
		time.setFrequency(data.timerFrequency);
		return true;
  }

  function HeapCreate(buffer, data) {
	  var ret = [];
	  if(!STREAM.readByte(buffer, ret))   { return false; } //id
	  if(!STREAM.readString(buffer, ret)) { return false; } //name
	  //begin out event
	  data.id   = ret[0].readUInt32BE(4);
	  data.name = ret[1];
	  return true;
  }

  function HeapDestroy(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) {return false;} //id

		data.id = ret[0].readUInt32BE(4);

		return true;
  }

  function HeapAddCore(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.readByte(buffer, ret)) { return false; } //pointer to core start
		if(!STREAM.readByte(buffer, ret)) { return false; } //size

		data.id      = ret[0].readUInt32BE(4);
		data.pointer = ret[1];
		data.size    = ret[2].readUInt32BE(4);

		return true;
	}

	function HeapRemoveCore(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.readByte(buffer, ret)) { return false; } //pointer to core start
		if(!STREAM.readByte(buffer, ret)) { return false; } //size

		data.id   	 = ret[0].readUInt32BE(4);
		data.pointer = ret[1];
		data.size 	 = ret[2].readUInt32BE(4);

		return true;
	}

  function HeapAllocate(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.readByte(buffer, ret)) { return false; } //pointer to start
		if(!STREAM.readByte(buffer, ret)) { return false; } //size

		//TODO check so allocation is not already made
		data.id      = ret[0].readUInt32BE(4);
		data.pointer = ret[1];
		data.size    = ret[2].readUInt32BE(4);

		return true;
  }

  function HeapFree(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) { return false; } //id
		if(!STREAM.readByte(buffer, ret)) { return false; } //pointer;

		data.id      = ret[0].readUInt32BE(4);
		data.pointer = ret[1];

		return true;
  }
} //Eventdecoder end
