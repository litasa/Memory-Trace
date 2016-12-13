var EventReader = new function() {

  this.oneEvent = function oneStepRead(buffer, currentState){
		var data = {};

	  buffer.setRollback();

		if(!BeginEvent(buffer, data)) { return null; }

		switch(data.event) {
			
			case(global.typeEnum.BeginStream) :
      	if(!BeginStream(buffer,data)) {
					return null;
				}
				//TODO -- check that magic number is correct etc
			break;

			case(global.typeEnum.HeapCreate) :
				if(!HeapCreate(buffer,data)) {
					return null;
				}
				currentState.createHeap(data);
			break;

			case(global.typeEnum.HeapDestroy) :
				if(!HeapDestroy(buffer,data)) {
					return null;
				}
				currentState.removeHeap(data);
			break;

			case(global.typeEnum.HeapAddCore) :
				if(!HeapAddCore(buffer,data)) {
					return null;
				}
				currentState.createCore(data);
			break;

			case(global.typeEnum.HeapRemoveCore) :
				if(!HeapRemoveCore(buffer,data)) {
					return null;
				}
				currentState.removeCore(data);
			break;

			case(global.typeEnum.HeapAllocate) :
				if(!HeapAllocate(buffer,data)) {
					return null;
				}
				currentState.createAllocation(data);
			break;

			case(global.typeEnum.HeapFree) :
				if(!HeapFree(buffer,data)) {
					return null;
				}
				currentState.removeAllocation(data);
			break;

			case(global.typeEnum.EndStream) :
				//TODO -- Check so that no leaks have happened
				//no additional data sent other than the Begin Event
			break;
			default:
				return null;
		}
		buffer.setRollback();

		return data;
  }

	function BeginEvent(buffer, data) {
		var ret = [];
    if(!STREAM.readByte(buffer, ret)) { return false; }  //code
    if(!STREAM.readByte(buffer, ret)) { return false; } //timestamp

	  data.event     = ret[0].readUInt32BE(4);
	  data.timestamp = ret[1];

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

		return true;
  }

  function HeapCreate(buffer, data) {
	  var ret = [];
	  if(!STREAM.readByte(buffer, ret))   { return false; } //id
	  if(!STREAM.readString(buffer, ret)) { return false; } //name
	  //begin out event
	  data.id     = ret[0].readUInt32BE(4);
	  data.name		= ret[1];
	  return true;
  }

  function HeapDestroy(buffer, data) {
		var ret = [];
		if(!STREAM.readByte(buffer, ret)) {return false;} //id

		data.id   = ret[0].readUInt32BE(4);

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
} //Eventreader end
