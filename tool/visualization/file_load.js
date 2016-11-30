var rem = require('electron').remote;
var dialog = rem.dialog;
var fs = require('fs');
var net = require('net');

function readFile(filepath){
    var client = new net.Socket();
    fs.readFile(filepath, function (err, data) {
          if(err){
              alert("An error ocurred reading the file :" + err.message);
              return;
          }

		      client.connect(server.port, server.address, function() {
			    client.end(data);
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
