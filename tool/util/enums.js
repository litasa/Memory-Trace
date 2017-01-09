Enum = {
  'BeginStream' : 1,
  'EndStream': 2,

  'HeapCreate': 18,
  'HeapDestroy': 19,
  'HeapAddCore': 20,
  'HeapRemoveCore': 21,
  'HeapAllocate': 22,
  'HeapFree': 23
};

function getEventAsString(event)
{
  var string = new String();
  string = "(" + event +")";
  if(event == 1){
    return string.concat("BeginStream");
  }  else if (event == 2) {
    return string.concat("EndStream");
  }   else if (event == 18) {
      return string.concat("HeapCreate");
  }  else if (event == 19) {
      return string.concat("HeapDestroy");
  }  else if (event == 20) {
      return string.concat("HeapAddCore");
  }  else if (event == 21) {
      return string.concat("HeapRemoveCore");
  }  else if (event == 22) {
      return string.concat("HeapAllocate");
  }   else if (event == 23) {
      return string.concat("HeapFree");
  }
  else {
    return string.concat("Unhandled Event");
  }
}
