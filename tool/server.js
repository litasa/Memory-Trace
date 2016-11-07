//object that handles the buffer

var BufferReader = new function() {
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
    if(!readHeaderData(header, buffer)) { return false; }

    if (header.event == global.typeEnum.BeginStream) {  //stream begin event
      //TODO -- check that magic number is correct etc
      readBeginStream(buffer,header);
    }
    else if (header.event == global.typeEnum.ModuleDump) { //module dump
        readModuleDump(buffer,header);
    }
    else if (header.event == global.typeEnum.HeapCreate) {
      readHeapCreate(buffer,header);
    }
    else if (header.event == global.typeEnum.HeapDestroy) {
      readHeapDestroy(buffer,header);
    }
    else if (header.event == global.typeEnum.HeapAddCore) {
      readHeapAddCore(buffer,header);
    }
    else if (header.event == global.typeEnum.HeapAllocate) {
      readHeapAllocate(buffer,header);
    }
    else if (header.event == global.typeEnum.HeapFree) {
      readHeapFree(buffer,header);
    }
    else if (header.event == global.typeEnum.EndStream) {
      //TODO -- Check so that no leaks have happened
    }
    //var totalMemory = getTotalMemory(global.HeapsAlive);
    //addChartData(meep, { x: header.timestamp, y: totalMemory});
  }
	
	function readHeaderData(header, buffer){
	var ret = [];
    if(!STREAM.read32Byte(ret, buffer)) { return false; }  //code
    if(!STREAM.read32Byte(ret, buffer))  { return false; } //scope
	
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
    
	if(!STREAM.read64Byte(ret, buffer)) { return false;}
	if(!STREAM.readString(ret, buffer)) { return false;}
	if(!STREAM.read32Byte(ret, buffer)) { return false;}
	if(!STREAM.read64Byte(ret, buffer)) { return false;}
	if(!STREAM.read64Byte(ret, buffer)) { return false;}
	
	beginStreamEvent.magicNumber       = ret[0];
	beginStreamEvent.platform          = ret[1];
	beginStreamEvent.pointerSize       = ret[2];
	beginStreamEvent.timerFrequency    = ret[3];
	beginStreamEvent.initCommonAddress = ret[4];
  }

  function readModuleDump(buffer, head) {
    //TODO -- save symData somewhere
	var tempSymData = [];
    while(true)
	{
		var dump = {};
		var ret = [];
		if(!STREAM.read32Byte(ret,buffer)) {return false;}
		if(ret.pop() == 0)
		{
			break;
		}
		
		if(!STREAM.readString(ret, buffer)) { return false; }
		if(!STREAM.read64Byte(ret, buffer)) { return false; }
		if(!STREAM.read64Byte(ret, buffer)) { return false; }
		dump.name = ret[0];
		dump.base = ret[1];
		dump.size = ret[2];
		//add to list
		tempSymData.push(dump);
	}
	//add to global modules
	global.Modules = tempSymData;
  }

  function readHeapCreate(buffer, head) {
	  var heapCreate = {};
	  var ret = [];
	  if(!STREAM.read32Byte(ret, buffer)) { return false; } //id
	  if(!STREAM.readStringIndex(ret,buffer)) { return false; } //name
	  //begin out event
	  heapCreate.head   = head;
	  heapCreate.id     = ret[0];
	  heapCreate.nameId = ret[1];
  }

  function readHeapDestroy(buffer, head) {
    var heapDestroy = {};
	var ret = [];
	if(!STREAM.read32Byte(ret,buffer)) {return false;} //id
	
	heapDestroy.head = head;
	heapDestroy.id   = ret[0];
  }

  function readHeapAddCore(buffer,head) {
    var heapAddCore = {};
	var ret = [];
	if(!STREAM.read32Byte(ret,buffer)) { return false; } //id
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer to core start
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //size
	
	heapAddCore.head = head;
	heapAddCore.id   = ret[0];
	heapAddCore.base = ret[1];
	heapAddCore.size = ret[2];
  }

  function readHeapAllocate(buffer, head) {
    var allocation = {};
	var ret = [];
	
	if(!STREAM.read32Byte(ret,buffer)) { return false; } //id
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer to start
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //size
	
	//TODO check so allocation is not already made	
	allocation.id = ret[0];
	allocation.pointer = ret[1];
	allocation.size = ret[2];
  }

  function readHeapFree(buffer,head) {
    var heapFree = {};
	var ret = [];
	
	if(!STREAM.read32Byte(ret,buffer)) { return false; } //id
	if(!STREAM.read64Byte(ret,buffer)) { return false; } //pointer;
	
	heapFree.head    = head;
	heapFree.id      = ret[0];
	heapFree.pointer = ret[1];
  }
}

var STREAM = new function () {
  
  
  global.Modules = [];

  this.read32Byte = function readByte(code, stream){
    var val = 0;
    var tempindex = stream.index;
    var mul = 1;
    do {
		if(tempindex == stream.data.length)
		{
			code = 0;
			return false;
		}
      var b = stream.data[tempindex++];
      val |= b*mul;
      mul <<= 7;
    } while (b < 0x80);

    val &= ~mul;

    stream.index = tempindex;
	code.push(val);
    return true;
  }

  this.read64Byte = function read64Byte(code, stream){
    var val = 0;
    var point = { high: 0, low: 0};
    var tempindex = stream.index;
    var mul = 1;
    do {
		if(tempindex == stream.data.length) {
			code = 0;
			return false;
		}
		var b = stream.data[tempindex++];
		val |= b*mul;
		mul <<= 7;
		if (mul > HIGH_THREASHOLD_64BIT) {
			point.low = val;
			mul = 1;
			val = 0;
		}
    } while (b < 128);
    val &= ~mul;
    if (point.low > 0) { //we have 64 bit
      point.high = val;
    }
    else {
      point.low = val;
    }
    stream.index = tempindex;
    code.push(point);
	return true;
  }
  
  this.readString = function readString(ret, stream)
  {
	  if(!this.read32Byte(ret, stream)) { return false; }
	  var index = ret.pop();
	  
	  if(index < global.SeenStrings.size)
	  {
		  string = global.SeenStrings.get(index);
		  return true;
	  }
	  
	  if(!this.read32Byte(ret, stream)) { return false; }
	  var length = ret.pop();
	  
	  var string = String("");
	  for(i=0; i < length; ++i)
	  {
		  if(stream.index + i == stream.data.length) { return false; }
		  
		  string += String.fromCharCode(stream.data[stream.index + i]);
	  }
	  
	  ++global.seenStringRollback;
		/* dunno what this is atm
		string shared;
      if (!m_StringCache.TryGetValue(data, out shared))
      {
        shared = data;
        m_StringCache.Add(data, data);
      }
		*/
	  global.SeenStrings.set(index,string);
	  stream.index += length;
	  ret.push(string);
	  return true;
  }
  
  this.readStringIndex = function readStringIndex(ret, stream)
  {
	  var temp = [];
	  if(!this.read64Byte(temp,stream)){ return false; }
	  var sequence = temp.pop();
	  
	  if(pointerLessVal(sequence, global.SeenStacks.size))
	  {
		  ret.push(sequence);
		  return true;
	  }
	  
	  if(!this.read32Byte(temp, stream)) { return false; }
	  var length = temp.pop();
	  var string = new String("");
	  for(i = 0; i < length; i++)
	  {
		  if(stream.index + i > stream.data.length) { return false; }
		  string += String.fromCharCode(stream.data[stream.index + i]);
	  }
	  
	  ++stream.seenStringRollback;
	  global.SeenStrings.set(sequence, string);
	  stream.index += length;
	  ret.push(global.SeenStrings.size - 1);
	  return true;
  }

  this.readBacktraceIndex = function readBacktraceIndex(ret, stream)
  {
	  if(!this.read32Byte(ret, stream))
	  {
		ret.push(-1);
		return false;
	  }
	  var index = ret.pop();
	  
	  if(index < global.SeenStacks.size)
	  {
		  ret.push(index);
		  return true;
	  }
	  
	  if(index != global.SeenStacks.size)
	  {
		  throw "Unexpected stack index"
	  }
	  
	  if(!this.read32Byte(ret,stream))
	  {
		  ret.push(-1);
		  return false;
	  }
	  var frame_count = ret.pop();
	  var frames = new Array();
	  
	  for(i = 0; i < frame_count; ++i)
	  {
		  if(!this.read64Byte(frames,stream))
		  {
			  ret.push(-1);
			  return false;
		  }
		  //add to metadata
		  /*
		  if (!MetaData.Symbols.Contains(frames[i]))
		  {MetaData.Symbols.Add(frames[i]);}
		  */
	  }
	  ++global.seenStringRollback;
	  global.SeenStacks.set(index, frames);
	  ret.push(index);
	  return true;
  }
};

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

function printConsole(buffer){
  for (var i = 0; i < buffer.length; i++) {
    var hexnum = buffer[i].toString(16);
    console.log(hexnum + " ")
  }
}

//Helperfunctions for printing
function printheader(header) {
  console.log("event is: " + this.getEventAsString(header.event));
  console.log("Timestamp is: " + header.timestamp);
}

function AllocationArrayToString(allocationArray){
  var string = new String;
  for (var i = 0; i < allocationArray.length; i++) {
    var all = "Pointer: " + pointerToHex(allocationArray[i].pointer) + " Size: " + allocationArray[i].size + "\n";
    string = string.concat(all);
  }
  return string;
}

total_data_handled = 0;
// server
var server = require('net').createServer(function (socket) {
    console.log("connected");
    var total_data_recieved = 0;
	
	var ringBuffersize = 128*1024;
	var ringBuffer = new Buffer(ringBuffersize);
	var ringStart = 0;
	var ringEnd = 0;
	
    socket.on('data', function (data) {
        var buffer = {
          data: Buffer.from(data,'hex'),
          index: 0,
          rollbackNeeded: false,
          rollback: 0
        };
		
        console.log("data recieved, Length: " + buffer.data.length);
        total_data_handled += data.length;
		
		var data_offset = 0;
		
		do{
			var ringspace = (ringBuffersize - (ringEnd - ringStart));
			
			ringspace = Math.min(buffer.data.length - data_offset, ringspace);
			
			var wpos = ringEnd;
			
			while(ringspace > 0){
				ringBuffer[wpos & (ringBuffersize -1 )] = buffer.data[data_offset++];
				--ringspace;
				++wpos;
			}
			
			ringEnd = wpos;
			
			BufferReader.oneEvent(ringBuffer);
			
			
		} while(data_offset < buffer.data.length)

        console.log("done with buffer" + total_data_handled);
    })
})
.listen(8080);


var pointerdata = new Array();
var id = 0;
function checkAllocationPointer(pointer)
{
  if (pointerdata.length < 2) {
    pointerdata.push(pointer);
    return;
  }
  if (pointerEquals(pointerdata[0],pointer) || pointerEquals(pointerdata[1],pointer)) {
    return;
  }
  console.log("Allocation Pointers not the same");
}

function getTotalMemory(heapsalive)
{
  var ret = 0;
  for (var i = 0; i < heapsalive.size; i++) {
    if (heapsalive.has(i)) {
    for (var j  = 0; j < heapsalive.get(i).cores.length; j++) {
      ret += heapsalive.get(i).getMemoryUsage();
      }
    }
  }
  return ret;
}

server.on('close', function() {
  console.log("connection closed, hmm");
});

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
  console.log('Address: '+ add + '\n socket: ' + server.address().port);
});
