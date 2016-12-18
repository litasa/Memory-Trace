const nativeExtension = require('../');
const BigNumber = require('bignumber.js');
const assert = require('assert');

var hex = "42413a1a0620";
var bin = [66,65,38,26,6,32];
var magic = Buffer.from("0300afbf00", 'hex')
var magic_reverse = Buffer.from("0300afbf00", 'hex')
var enc_magic = Buffer.from("03003c7d8b", 'hex')

testFunc = function(functionToTest, data,times) {
    var start1 = Date.now();
    for(var i = 0; i < times; ++i) {
        functionToTest(data);
    }
    var stop1 = Date.now();
    return stop1-start1;
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
    while(s.length < 64) { s = '0' + s;}
    for(var i = 0; i < 8; ++i) {
        ret[i] = parseInt(s.slice(i*8, i*8+8), 2);
    }
    ret.swap64();
    return ret;
}

console.log("native: " + testFunc(nativeExtension.decode, enc_magic, 10000000));
console.log("Decode: " + testFunc(decodeBuffer, enc_magic, 10000000));

//assert.deepEqual(nativeExtension.decode(enc_magic), magic);
//assert.deepEqual(decodeBuffer(enc_magic), magic);