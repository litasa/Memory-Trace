
var STREAM = new function () {


  global.Modules = [];

  this.readByte = function readByte(code, stream){
    var val = 0;
    var mul = 1;
    do {
      var b = stream.getNextValue();

		if(stream.readOverflow())
		{
			code = 0;
			return false;
		}
      val |= b*mul;
      mul <<= 7;
    } while (b < 0x80);

    val &= ~mul;
    code.push(val);
    return true;
  }

  this.read64Byte = function read64Byte(code, stream){
    var val = 0;
    var mul = 1;
    var out = [];
    do {
  		var b = stream.getNextValue();
  		if(stream.readOverflow()) {
  			code = 0;
  			return false;
  		}
      out.push(b);
    } while (b < 128);
    out[out.length -1] -= 0x80;

    code.push(out.reverse());

	   return true;
  }

  this.readString = function readString(ret, stream)
  {
	  if(!this.readByte(ret, stream)) { return false; }
	  var index = ret.pop();

	  if(index < global.SeenStrings.size)
	  {
		  string = global.SeenStrings.get(index);
		  return true;
	  }

	  if(!this.readByte(ret, stream)) { return false; }
	  var length = ret.pop();

	  var string = String("");
	  for(i=0; i < length; ++i)
	  {
		  if(stream.readOverflow()) { return false; }

		  string += String.fromCharCode(stream.getNextValue());
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
	  ret.push(string);

	  return true;
  }

  this.readStringIndex = function readStringIndex(ret, stream)
  {
	  var temp = [];
	  if(!this.read64Byte(temp,stream)){ return false; }
	  var sequence = temp.pop();

	  if(hexToInt(sequence) < global.SeenStacks.size)
	  {
		  ret.push(sequence);
		  return true;
	  }

	  if(!this.readByte(temp, stream)) { return false; }
	  var length = temp.pop();
	  var string = new String("");
	  for(i = 0; i < length; i++)
	  {
		  if(stream.readOverflow()) { return false; }
		  string += String.fromCharCode(stream.getNextValue());
	  }

	  ++stream.seenStringRollback;
	  global.SeenStrings.set(sequence, string);
	  ret.push(global.SeenStrings.size - 1);

	  return true;
  }

  this.readBacktraceIndex = function readBacktraceIndex(ret, stream)
  {
	  if(!this.readByte(ret, stream))
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

	  if(!this.readByte(ret,stream))
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
