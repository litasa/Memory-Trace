(function () {
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

  global.SeenStacks = new Set();

  global.HeapsAlive = new Map();
  global.HeapsDead = new Map();
}());







// server
var server = require('net').createServer(function (socket) {

    console.log("connected");
    socket.on('data', function (data) {
        var buffer = {
          data: new Buffer(data,'hex'),
          index: 0
        };

        console.log("data recieved, Length: " + buffer.data.length);
        while (buffer.index < buffer.data.length) {
          var header = {
            event: null,
            scope: null,
            timestamp: null,
            callstackId: null
          };
          header.event = readByte(buffer);
          header.scope = readByte(buffer);
          if (header.scope != 0) {
            header.scope = readString(buffer);
          }
          header.timestamp = readByte(buffer);

          //checking to see if we are doing a second run of the same program. If that is the case remove all saved data and begin from scratch
          var check = peakByte(buffer);
          if (check.val == 0) {
            console.clear();
            console.log("New run detected. Clearing: SeenStacks, HeapsAlive, HeapsDead");
            global.SeenStacks.clear();
            global.HeapsAlive.clear();
            global.HeapsDead.clear();
          }
          header.callstackId = readCallstackData(buffer);

          printheader(header);

          if (header.event == global.typeEnum.BeginStream) {  //stream begin event
            //TODO check that magic number is correct etc
            var magicNumber = read64Bit(buffer);
            var platform = readString(buffer);
            var pointerSize = readByte(buffer);
            var timerFrequency = read64Bit(buffer);
            var initCommonAddress = readPointer(buffer);
          }
          else if (header.event == global.typeEnum.ModuleDump) { //module dump
              //TODO save symData somewhere
              var symData = readSymbols(buffer);
          }
          else if (header.event == global.typeEnum.HeapCreate) {
            var heapId = readByte(buffer);
            var heapName = readString(buffer);
            if (global.HeapsAlive.has(heapId)) {
              console.log("Error, created a heap that already exists" + heapId);
              return;
            }
            global.HeapsAlive.set(heapId,new Heap(heapName,header.timestamp,header.callstackId));
          }
          else if (header.event == global.typeEnum.HeapDestroy) {
            var heapId = readByte(buffer);
            if (global.HeapsAlive.has(heapId)) {
              var heapdata = global.HeapsAlive.get(heapId);
              var destroyData = heapdata.Destroy();
              if (destroyData.destroyed == false) {
                console.log("Could not destroy heap " + heapId + " with name " + destroyData.name +
              " because core: " + destroyData.core + " still contains \n" + AllocationArrayToString(destroyData.allocList));
              }
              else {
                heapdata.death = header.timestamp;
                global.HeapsAlive.delete(heapId);
                global.HeapsDead.set(heapId,heapdata);
              }
            }
            else {
              console.log("Trying to Destroy non existing heap");
            }
          }
          else if (header.event == global.typeEnum.HeapAddCore) {
            var heapId = readByte(buffer);
            var startPos = readPointer(buffer);
            var size = readByte(buffer);
            if (global.HeapsAlive.has(heapId)) {
              var heapdata = global.HeapsAlive.get(heapId);
              heapdata.AddCore(new Core(startPos, size, header.timestamp, header.callstackId));
            }
            else {
              console.log("Trying to add a Core to non-existing heap: " + heapId);
            }
          }
          else if (header.event == global.typeEnum.HeapAllocate) {
            var heapId = readByte(buffer);
            var startPos = readPointer(buffer);
            var size = readByte(buffer);
            if (global.HeapsAlive.has(heapId)) {
              var heapdata = global.HeapsAlive.get(heapId);
              heapdata.Allocate(new Allocation(startPos, size, header.timestamp, header.callstackId));
            }
          }
          else if (header.event == global.typeEnum.HeapFree) {
            var heapId = readByte(buffer);
            var startPos = readPointer(buffer);
            if (global.HeapsAlive.has(heapId)) {
              var heapdata = global.HeapsAlive.get(heapId);
              heapdata.DeallocateFromPointer(startPos);
            }
          }
          else if (header.event == global.typeEnum.EndStream) {
            //TODO Check so that no leaks have happened
          }
        } //All events registred. Buffer is empty
        lineChart.update();
		chart.render();
    })
})
.listen(8080);

server.on('close', function() {
  console.log("connection closed, hmm");
});

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
  console.log('Address: '+ add + '\n socket: ' + server.address().port);
});

function printConsole(buffer){
  for (var i = 0; i < buffer.length; i++) {
    var hexnum = buffer[i].toString(16);
    console.log(hexnum + " ")
  }
}

function readByte(stream){
  var val = 0;
  var tempindex = stream.index;
  var mul = 1;
  do {
    var b = stream.data[tempindex++];
    val |= b*mul;
    mul <<= 7;
  } while (b < 128);

  val &= ~mul;
  stream.index = tempindex;
  return val;
}

function peakByte(stream) {
  var val = 0;
  var tempindex = stream.index;
  var mul = 1;
  do {
    var b = stream.data[tempindex++];
    val |= b*mul;
    mul <<= 7;
  } while (b < 128);

  val &= ~mul;
  return { val: val, index: tempindex};
}

const HIGH_THREASHOLD_64BIT = 2097152;

function readPointer(stream){
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
  stream.index = tempindex;
  return point;
}

function read64Bit(stream){
  return readPointer(stream);
}

function readCallstackData(buffer)
{
  var sequence = readByte(buffer);
  if (sequence < global.SeenStacks.size) {
    return sequence;
  }
  var numberOfFrames = readByte(buffer);
  var frameInfo = new Array(numberOfFrames);
  for (var i = 0; i < numberOfFrames; i++) {
    frameInfo[i] = readByte(buffer);
    //TODO Add Metadata Symbol Contains Thingie
  }
  global.SeenStacks.add(frameInfo);
  return sequence;
}

function readString(buffer)
{
  //get the sequence number to know if it has been used before
  var sequence = readByte(buffer);
  //TODO handle reuse if multiple strings used
  var stringLength = readByte(buffer);
  var platform = new String("");
  for (var i = buffer.index; i < buffer.index + stringLength; i++) {
    platform += String.fromCharCode(buffer.data[i]);
  }
  buffer.index += stringLength;
  return platform;
}

function readSymbols(buffer)
{
  var symData = []
  while (bit != 0) {
    var bit = readByte(buffer);
    if (bit == 0) {
      return;
    };
    symData.push({
      name: readString(buffer),
      base: readByte(buffer),
      size: readByte(buffer)
    });
  }
  return symData;
}

function printheader(header)
{
  console.log("event is: " + getEventAsString(header.event));
  console.log("Timestamp is: " + header.timestamp);
}

function AllocationArrayToString(allocationArray){
  var string = new String;
  for (var i = 0; i < allocationArray.length; i++) {
    var all = "Pointer: " + string64bitAsHex(allocationArray[i].pointer) + " Size: " + allocationArray[i].size + "\n";
    string = string.concat(all);
  }
  return string;
}

function string64bitAsHex(val){
  return val.high.toString(16) + val.low.toString(16);
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
