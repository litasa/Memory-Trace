var BigNumber = require('bignumber.js');
var STREAM = new function () {
  
  global.Modules = [];

  this.readByte = function read64Byte(buffer, ret){
    var count = 0;
    var out = Buffer.alloc(10);
    do {
  		var b = buffer.next();
  		if(b === null) {
  			ret = 0;
  			return false;
  		}
      out[count++] = b;
    } while (b < 128);
    out[count -1] -= 0x80;
    var big = new BigNumber("0x" + decodeBuffer(out).toString('hex'),16);
    ret.push(big);
	  return true;
  }

	decodeBuffer = function decodeBuffer(buffer) {
    //This should probably be done in cpp as a Modules
    //it is kinda slow
    var ret = Buffer.allocUnsafe(8)
    var s = "";
    for(var i = 0; i < buffer.length; ++i) {
        var tmp = buffer[i].toString(2);
        while(tmp.length < 8) tmp = '0' + tmp;
        tmp = tmp.substr(1);
        s = tmp + s;
    }
    //s is length 70 make it 64
    while(s.length > 64) { s = s.substr(1); }
    for(var i = 0; i < 8; ++i) {
        ret[i] = parseInt(s.slice(i*8, i*8+8), 2);
    }

    return ret;
	}

  this.readString = function readString(buffer, ret)
  {
	  if(!this.readByte(buffer, ret)) { return false; }
	  var length = ret.pop().toNumber();

	  var string = String("");
	  for(i=0; i < length; ++i)
	  {
      var b = buffer.next();
      if(b === null) {
        ret = 0;
        return false;
      }
		  string += String.fromCharCode(b);
	  }
	  ret.push(string);

	  return true;
  }
};
