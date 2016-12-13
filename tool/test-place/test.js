var orig_hex = "bfaf0003";
var enc_hex = "03003c7d8b";
var enc_buff = Buffer.from(enc_hex, 'hex');
var orig_buff = Buffer.from(orig_hex, 'hex');

function reverse(s) {
  return s.split('').reverse().join('');
}

testFunc = function(functionToTest, data,times) {
    var start1 = performance.now();
    for(var i = 0; i < times; ++i) {
        functionToTest(data);
    }
    var stop1 = performance.now();
    return stop1-start1;
}

read64Byte = function read64Byte(buffer){
    //this is a dirty function and should probably be done in c++
    //as an addon
    var count = 0;
    var out = Buffer.alloc(8);
    do {
  		var b = buffer[count];
  		if(b === null) {
  			ret = 0;
  			return false;
  		}
      out[count++] = b;
    } while (b < 128);
    out[count -1] -= 0x80;
    out = decodeBuffer(out);
    return out;
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

var dec_buff = read64Byte(enc_buff);
console.log("testing read 64 Byte: " + testFunc(read64Byte,enc_buff,100));
//console.log("buffer reverse: " + testFunc(reverseBuffer,buff,times))
//console.log("arr reverse " + testFunc(reverseArray,bin,times))