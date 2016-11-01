//object that handles the buffer

var STREAM = new function () {
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

  this.readByte = function(stream){
    var val = 0;
    var tempindex = stream.index;
    var mul = 1;
    do {
      var b = stream.data[tempindex++];
      val |= b*mul;
      mul <<= 7;
    } while (b < 128);

    val &= ~mul;
    if (tempindex > stream.data.length) {
      stream.rollbackNeeded = true;
      throw "PROBLEM, READ TOO LONG IN BUFFER";
    }
    stream.index = tempindex;
    return val;
  }

  this.peakByte = function(stream) {
    var val = 0;
    var tempindex = stream.index;
    var mul = 1;
    do {
      var b = stream.data[tempindex++];
      val |= b*mul;
      mul <<= 7;
    } while (b < 128);
    if (tempindex > stream.data.length) {
      console.log("PROBLEM, PEAKED TOO LONG IN BUFFER")
    }
    val &= ~mul;
    return { val: val, index: tempindex};
  }



  this.readPointer = function(stream){
    var val = 0;
    var point = { high: 0, low: 0};
    var tempindex = stream.index;
    var mul = 1;
    do {
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
    if (tempindex > stream.data.length) {
      stream.rollbackNeeded = true;
      throw "PROBLEM, READ TOO LONG IN BUFFER";
    }
    stream.index = tempindex;
    return point;
  }

  this.peakPointer = function(stream){
    var val = 0;
    var point = { high: 0, low: 0};
    var tempindex = stream.index;
    var mul = 1;
    do {
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
    if (tempindex > stream.data.length) {
      console.log("PROBLEM, READ TOO LONG IN BUFFER")
    }
    return point;
  }

  this.read64Bit = function(stream){
    return this.readPointer(stream);
  }

  this.readCallstackData = function(buffer)
  {
    var sequence = this.readByte(buffer);
    if (sequence < global.SeenStacks.size) {
      return sequence;
    }
    var numberOfFrames = this.readByte(buffer);
    var frameInfo = new Array(numberOfFrames);
    for (var i = 0; i < numberOfFrames; i++) {
      frameInfo[i] = this.readByte(buffer);
      //TODO Add Metadata Symbol Contains Thingie
    }
    global.SeenStacks.set(sequence, frameInfo);
    return sequence;
  }

  this.readString = function(buffer)
  {
    //get the sequence number to know if it has been used before
    var sequence = this.readByte(buffer);
    if (sequence < global.SeenStrings.size) {
      return global.SeenStrings.get(sequence);
    }
    var stringLength = this.readByte(buffer);
    var string = new String("");
    for (var i = buffer.index; i < buffer.index + stringLength; i++) {
      string += String.fromCharCode(buffer.data[i]);
    }
    global.SeenStrings.set(sequence, string);
    buffer.index += stringLength;
    return string;
  }

  this.readSymbols = function(buffer)
  {
    var symData = []
    while (bit != 0) {
      var bit = this.readByte(buffer);
      if (bit == 0) {
        return;
      };
      symData.push({
        name: this.readString(buffer),
        base: this.readByte(buffer),
        size: this.readByte(buffer)
      });
    }
    return symData;
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

// server
var server = require('net').createServer(function (socket) {

    function readHeaderData(buffer){
      var header = {
        event: null,
        scope: null,
        timestamp: null,
        callstackId: null
      };

      header.event = STREAM.readByte(buffer);
      header.scope = STREAM.readByte(buffer);
      if (header.scope != 0) {
        header.scope = STREAM.readString(buffer);
      }
      header.timestamp = STREAM.readByte(buffer);
      header.callstackId = STREAM.readCallstackData(buffer);
      return header;
    }

    function readBeginStream(buffer, head){
        //TODO check that magic number is correct etc
      var magicNumber = STREAM.read64Bit(buffer);
      var platform = STREAM.readString(buffer);
      var pointerSize = STREAM.readByte(buffer);
      var timerFrequency = STREAM.read64Bit(buffer);
      var initCommonAddress = STREAM.readPointer(buffer);
          sanityCheck(STREAM.readByte(buffer), head.event);
    }

    function readModuleDump(buffer, head) {
      //TODO save symData somewhere
      var symData = STREAM.readSymbols(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
    }

    function readHeapCreate(buffer, head) {
      var heapId = STREAM.readByte(buffer);
      var heapName = STREAM.readString(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
      if (!global.HeapsAlive.has(heapId)) {
        global.HeapsAlive.set(heapId,new Heap(heapName,head.timestamp,head.callstackId));
      }
      else {
        console.log("Error, created a heap that already exists" + heapId);
      }
    }

    function readHeapDestroy(buffer, head) {
      var heapId = STREAM.readByte(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
      if (global.HeapsAlive.has(heapId)) {
        var heapdata = global.HeapsAlive.get(heapId);
        var destroyData = heapdata.Destroy();
        if (destroyData.destroyed == true) {
          head.death = head.timestamp;
          global.HeapsAlive.delete(heapId);
          global.HeapsDead.set(heapId,heapdata);
        }
        else {
          console.log("Could not destroy heap " + heapId + " with name " + destroyData.name +
        " because core: " + destroyData.core + " still contains \n" + AllocationArrayToString(destroyData.allocList));
        }
      }
      else {
        console.log("Trying to Destroy non existing heap");
      }
    }

    function readHeapAddCore(buffer,head) {
      var heapId = STREAM.readByte(buffer);
      var startPos = STREAM.readPointer(buffer);
      var size = STREAM.readByte(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
      if (global.HeapsAlive.has(heapId)) {
        var heapdata = global.HeapsAlive.get(heapId);
        heapdata.AddCore(new Core(startPos, size, head.timestamp, head.callstackId));
      }
      else {
        console.log("Trying to add a Core to non-existing heap: " + heapId);
      }
    }

    function readHeapAllocate(buffer, head) {
      var heapId = STREAM.readByte(buffer);
      var pointTemp = STREAM.peakPointer(buffer);
      checkAllocationPointer(pointTemp);
      var startPos = STREAM.readPointer(buffer);
      var size = STREAM.readByte(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
      if (global.HeapsAlive.has(heapId)) {
        var heapdata = global.HeapsAlive.get(heapId);
        heapdata.Allocate(new Allocation(startPos, size, head.timestamp, head.callstackId));
      }
      else {
        console.log("Trying to allocate to non-existing heap: " + heapId);
      }
    }

    function readHeapFree(buffer,head) {
      var heapId = STREAM.readByte(buffer);
      var startPos = STREAM.readPointer(buffer);
      sanityCheck(STREAM.readByte(buffer), head.event);
      if (global.HeapsAlive.has(heapId)) {
        var heapdata = global.HeapsAlive.get(heapId);
        heapdata.DeallocateFromPointer(startPos);
      }
      else {
        console.log("Trying to deallocate from non-existing heap: " + heapId);
      }
    }

    function sanityCheck(sanity ,code) {
      if (sanity != code) {
        throw "SanityCheck not valid";
      }
    }

    console.log("connected");

    socket.on('data', function (data) {
        var buffer = {
          data: new Buffer(data,'hex'),
          index: 0,
          rollbackNeeded: false,
          rollback: 0
        };
        try {
        console.log("data recieved, Length: " + buffer.data.length);

        while (buffer.index < buffer.data.length) {

          buffer.rollback = buffer.index;
          var header = readHeaderData(buffer);

          if (header.event == global.typeEnum.BeginStream) {  //stream begin event
            //TODO check that magic number is correct etc
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
            //TODO Check so that no leaks have happened
          }
          //var totalMemory = getTotalMemory(global.HeapsAlive);
          //addChartData(meep, { x: header.timestamp, y: totalMemory});
        } //All events registred. Buffer is empty

        console.log("done with buffer");
      } catch (e) {
        console.log(e + " at: " + buffer.index);
      }
      finally {

      }
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
