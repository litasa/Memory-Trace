var rem = require('electron').remote;
var dialog = rem.dialog;
var fs = require('fs');
var net = require('net');

function readFile(filepath){
    fs.readFile(filepath, function (err, data) {
          if(err){
              alert("An error ocurred reading the file :" + err.message);
              return;
          }
          var client = new net.Socket();
		  client.connect(8080, server.address(), function() {
			client.write(data);
		  });
    });
}

function saveFile(filepath, data) {
	fs.writeFile(filepath,data, (err) => {
		if(err) throw err;
		console.log("Saved " + filepath );
	});
}

document.getElementById('select-file').addEventListener('click',function(){
                dialog.showOpenDialog(function (fileNames) {
                    if(fileNames === undefined){
                        console.log("No file selected");
                    }else{
                        readFile(fileNames[0]);
                    }
                });
            },false);
