var nativeExtension = require('../');
var assert = require('assert');


describe('native extension', function() {
  it('Decode one value', function() {
    var enc_magic = Buffer.from("03003c7d8b", 'hex')
    var magic = Buffer.from("0300afbf00", 'hex')
    assert.deepEqual(nativeExtension.unpackBuffer(enc_magic), magic);
  });

  it('Encode one value', function() {
    var enc_magic = Buffer.from("03003c7d8b", 'hex')
    var magic = Buffer.from("0300afbf00", 'hex')
    assert.deepEqual(nativeExtension.encodeValue(magic), enc_magic);
  });
/*
  it('Decode header', function() {
    assert.deepEqual(nativeExtension.getHeader(enc_buffer), {'id': 1})
  });
  */
});