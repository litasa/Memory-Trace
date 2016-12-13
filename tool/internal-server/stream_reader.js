
var STREAM = new function () {
  
  global.Modules = [];

  this.readByte = function read64Byte(buffer, ret){
    var count = 0;
    var out = Buffer.alloc(8);
    do {
  		var b = buffer.next();
  		if(b === null) {
  			ret = 0;
  			return false;
  		}
      out[count++] = b;
    } while (b < 128);
    out[count -1] -= 0x80;
    ret.push(decodeBuffer(out));
	  return true;
  }

	decodeBuffer = function decodeBuffer(buffer) {
    var s = "";
    for(var i = 0; i < buffer.length; ++i) {
        var tmp = buffer[i].toString(2);
        while(tmp.length < 8) tmp = '0' + tmp;
        tmp = tmp.substr(1);
        s = tmp + s;
    }
    //add the last 8 zeroes
    s = "00000000" + s;
    for(var i = 0; i < buffer.length; ++i) {
        buffer[i] = parseInt(s.slice(i*8, i*8+8), 2);
    }
    
    return buffer;
	}

  this.readString = function readString(buffer, ret)
  {
	  if(!this.readByte(buffer, ret)) { return false; }
	  var length = ret.pop().readUInt32BE(4);;

	  var string = String("");
	  for(i=0; i < length; ++i)
	  {
		  string += String.fromCharCode(buffer.next());
	  }
	  ret.push(string);

	  return true;
  }
};