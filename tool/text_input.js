var rem = require('electron').remote; 
var dialog = rem.dialog;
var fs = require('fs');

function readFile(filepath){
    fs.readFile(filepath, 'utf-8', function (err, data) {
          if(err){
              alert("An error ocurred reading the file :" + err.message);
              return;
          }
          // Change how to handle the file content
          console.log("The file content is : " + data);
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