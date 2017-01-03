var nativeExtension = require('./');
const fs = require('fs');

console.log("testing")
var obj = nativeExtension.Decoder();
    var file_name = "./30_sec_run.bin";
      fs.readFile(file_name, function (err, data) {
        if(err){
          throw "An error ocurred reading the file :" + err.message;
        }
        console.log("inputting " + data.length + " amount of data")
        
        obj.unpackStream(data, file_name);

        console.log("Alll done!! :D");
});