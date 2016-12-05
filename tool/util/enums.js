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
