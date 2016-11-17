
HIGH_THREASHOLD_64BIT = 2097152;
LOW_BIT_THREASHOLD = 268435455;
//Under assumption that size is less than MAX_SAFE_INTEGER
function addSizeToPointer(pointer, size){
  var ret = { high: pointer.high, low: pointer.low};
  if (ret.low + size < LOW_BIT_THREASHOLD) {
    ret.low += size;
  }
  else {
    ret.high++;
    var rest = LOW_BIT_THREASHOLD - ret.low + 1;
    ret.low = size - rest;
  }
  return ret;
}

function pointerLE(lhs,rhs)
{
  if (lhs.high < rhs.high) {
    return true;
  }
  else if (lhs.high == rhs.high) {
    if (lhs.low <= rhs.low) {
      return true;
    }
  }
    return false;
}

function pointerLessVal(pointer,val)
{
	if(pointer.high != 0)
	{
		return false;
	}
	if(pointer.low >= val)
	{
		return false;
	}
	return true;
}

function pointerGE(lhs,rhs)
{
  if (lhs.high > rhs.high) {
    return true;
  }
  else if (lhs.high == rhs.high) {
    if (lhs.low >= rhs.low) {
      return true;
    }
  }
    return false;
}

function pointerBlockInside(lowAddress,highAddress,pointer, size)
{
    return (pointerLE(lowAddress,pointer)) && (pointerGE(highAddress,addSizeToPointer(pointer,size)));
}

function pointerEquals(pointerA, pointerB)
{
  if (pointerA.low == pointerB.low) {
    if (pointerA.high == pointerB.high) {
      return true;
    }
  }
  return false;
}

function pointerToHex(pointer){
  return "0x00" + zeroPadHex(pointer.high.toString(16)) + zeroPadHex(pointer.low.toString(16));
}

function zeroPadHex(hex){
  var len = hex.length;
  var zeroPad = "";
  for (var i = len; i < 7; i++) {
    zeroPad += "0";
  }
  return zeroPad + hex;
}
