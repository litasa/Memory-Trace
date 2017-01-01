var nativeExtension = require('./');
const fs = require('fs');

console.log("testing")
var obj = nativeExtension.Decoder();
      fs.readFile("./almost_minimal_test.bin", function (err, data) {
        if(err){
          throw "An error ocurred reading the file :" + err.message;
        }
        console.log("inputting " + data.length + " amount of data")
        // for(var i = 0; i < data.length; ++i) {
        //   process.stdout.write(data[i] + " ");
        //   if(i % 10 == 0) {
        //     process.stdout.write("\n");
        //   }
        // }
        
        obj.unpackStream(data);
});