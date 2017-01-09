var nativeExtension = require('../');
var assert = require('assert');
const fs = require('fs');


describe('native extension', function() {
  it('Decode one value', function() {
    var enc_magic = Buffer.from("03003c7d8b", 'hex')
    var magic = Buffer.from("0300afbf00", 'hex')
    assert.deepEqual(nativeExtension.decodeValue(enc_magic), magic);
  });

  it('Encode one value', function() {
    var enc_magic = Buffer.from("03003c7d8b", 'hex')
    var magic = Buffer.from("0300afbf00", 'hex')
    assert.deepEqual(nativeExtension.encodeValue(magic), enc_magic);
  });

  it('Encode one Number', function() {
    for(var i = 0; i < 0x80; ++i) {
      assert.deepEqual(nativeExtension.encodeNumber(i), i + 0x80);
    }
  });

  it('encode string', function() {
    var buff = Buffer.alloc(1);
    buff[0] = nativeExtension.encodeNumber(3);
    var buff2 = Buffer.from("hej");
    var res = Buffer.concat([buff,buff2])
    assert.deepEqual(nativeExtension.encodeString("hej"), res);
  });

  it('decode string', function() {
    var buff = Buffer.alloc(1);
    buff[0] = nativeExtension.encodeNumber(3);
    var buff2 = Buffer.from("hej");
    var res = Buffer.concat([buff,buff2])
    assert.deepEqual(nativeExtension.decodeString(res), "hej");
  });

  it('Encode then decode provides same value', function() {
    var enc_magic = Buffer.from("03003c7d8b", 'hex')
    var magic = Buffer.from("0300afbf00", 'hex')
    var dec = nativeExtension.decodeValue(nativeExtension.encodeValue(magic));
    assert.deepEqual(dec,magic);
  });
  /*
  it('Decode header', function() {
    assert.deepEqual(nativeExtension.getHeader(enc_buffer), {'id': 1})
  });
  */
}); // describe native extension

describe('Ringbuffer test', function() {

  it('populating buffer, no overflow populating', function() {
    
    var obj = nativeExtension.RingBuffer(8);
    var buff_res = Buffer.alloc(8);
    
    var buff = Buffer.from("456");
    assert.equal(obj.populate(buff), 0); //456
    buff.copy(buff_res);
    assert.deepEqual(obj.getBuffer(), buff_res);
    assert.equal(obj.getWritePos(), 3); //write position should be at 3
    assert.equal(obj.getUnread(), 3); //number of unread should be 3

    assert.equal(obj.populate(Buffer.from("8")), 0); //4568
    Buffer.from("8").copy(buff_res, 3);
    assert.deepEqual(obj.getBuffer(), buff_res);
    assert.equal(obj.getWritePos(), 4); //write position should be at 4
    assert.equal(obj.getUnread(), 4);

    assert.equal(obj.populate(Buffer.from("4568")), 0); //45684568
    Buffer.from("4568").copy(buff_res,4);
    assert.deepEqual(obj.getBuffer(), buff_res);
    assert.equal(obj.getWritePos(), 0); //write position should be at 0
    assert.equal(obj.getUnread(), 8);
  });

  it('populating buffer, overflow populating', function() {
    var obj = nativeExtension.RingBuffer(8);
    var buff_res = Buffer.alloc(8);
    Buffer.from("4567891").copy(buff_res);
    assert.equal(obj.populate(Buffer.from("4567891")), 0); //4567891
    assert.deepEqual(obj.getBuffer(), buff_res);
    assert.equal(obj.getReadPos(), 0);
    assert.equal(obj.getWritePos(), 7); //write position should be at 7
    assert.equal(obj.getUnread(), 7);

    assert.equal(obj.populate(Buffer.from("abc")), 2) //2 items unpopulated
    Buffer.from("a").copy(buff_res,7);
    assert.deepEqual(obj.getBuffer(), buff_res);
    assert.equal(obj.getReadPos(), 0);
    assert.equal(obj.getWritePos(), 0); //write position should be at 0
    assert.equal(obj.getUnread(), 8);
  });

  it('reading test', function() {
    var obj = nativeExtension.RingBuffer(8);
    var buff_res = Buffer.alloc(8);
    Buffer.from("45678913").copy(buff_res);
    obj.populate(Buffer.from("45678913")); //45678913
    for(var i = 0; i < buff_res.length; ++i) {
      assert.equal(obj.getUnread(), buff_res.length - i);
      assert.equal(obj.getReadPos(), i);
      assert.equal(obj.readNext(), buff_res[i]);
      assert.equal(obj.getWritePos(), 0);
    }
    //trying to read past number of unread
    assert.equal(obj.getUnread(), 0);
    assert.equal(obj.getReadPos(), 8);
    assert.deepEqual(obj.readNext(), null);
    assert.equal(obj.getReadPos(), 8);
    assert.equal(obj.getUnread(), 0);

   });

  it('reading then populating ringbuffer', function() {
    var obj = nativeExtension.RingBuffer(8);
    var buff_res = Buffer.alloc(8);
    Buffer.from("45678913").copy(buff_res);
    obj.populate(Buffer.from("45678913")); //45678913

    for(var i = 0; i < 4; ++i) {
      obj.readNext();
    }
    assert.equal(obj.getReadPos(), 4);
    assert.equal(obj.getUnread(), 4);

    // Buffer.from("123").copy(buff_res);
    // assert.equal(obj.populate(Buffer.from("123")), 0); //123789123
    // assert.deepEqual(obj.getBuffer(), buff_res);
    // assert.equal(obj.getReadPos(), 4);
    // assert.equal(obj.getUnread(), 7);
    // assert.equal(obj.getWritePos(),3);
  });

  it('testing rollback', function() {
    var obj = nativeExtension.RingBuffer(8);
    var buff_res = Buffer.alloc(8);
    Buffer.from("456789").copy(buff_res);
    obj.populate(Buffer.from("456789")); //456789
    obj.readNext();
    obj.setRollback();
    for(var i = 0; i < 4; ++i) {
      obj.readNext();
    }
    assert.equal(obj.getReadPos(), 5);
    assert.equal(obj.getUnread(), 1);
    assert.equal(obj.getWritePos(), 6);

    obj.doRollback();
    assert.equal(obj.getReadPos(), 1);
    assert.equal(obj.getUnread(), 5);
    assert.equal(obj.getWritePos(), 6);
  });
}); //describe Ringbuffer test