var nativeExtension = require('./');
const fs = require('fs');

console.log("testing")
var obj = nativeExtension.Decoder();
      fs.readFile("./hejhej.bin", function (err, data) {
        if(err){
          throw "An error ocurred reading the file :" + err.message;
        }
        console.log("inputting " + data.length + " amount of data")
        
        obj.unpackStream(data);
});