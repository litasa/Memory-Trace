//Under assumption that size is less than MAX_SAFE_INTEGER
function addSizeToPointer(pointer, size){
  var ret = { high: pointer.high, low: pointer.low};
  if (ret.low + size < Number.MAX_SAFE_INTEGER) {
    ret.low += size;
    return ret;
  }
  else {
    //TODO If size overflows the pointer
    console.log("PROBLEMS: Pointer and size overflows into high values")
  }
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
  return pointer.high.toString(16) + pointer.low.toString(16);
}