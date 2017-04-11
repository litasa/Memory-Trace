var rem = require('electron').remote;
var dialog = rem.dialog;
var fs = require('fs');
var currentStatus = require("./status.js")
var visualization = require("./visualization.js")
var send_to = require("../util/send_to.js")

function readFile(filepath){
    var client = new net.Socket();
    fs.readFile(filepath, function (err, data) {
        if(err){
            alert("An error ocurred reading the file :" + err.message);
            return;
        }
        if(server === null) {
            console.log('No server to connect to')            
        }
        else {
            client.connect(Window.server.port, Window.server.address, function() {
		        client.end(data);
		    });
        }
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
        }
        else {
            readFile(fileNames[0]);
        }
    });
},false);

document.getElementById('Unpack').addEventListener('click',function(){
    dialog.showOpenDialog(function (fileNames) {
        if(fileNames === undefined){
            console.log("No file selected");
        }
        else {
            fs.readFile(fileNames[0], function (err, data) {
                if(err){
                    alert("An error ocurred reading the file :" + err.message);
                    return;
                }
                currentStatus.SetMessage("Converting " + fileNames[0] + " to *.CSV")
                Window.decoder.saveAsCSV(data, fileNames[0] + ".csv");
                currentStatus.SetMessage("Converted " + fileNames[0] + " successfully to CSV")
            });
        }
    });
},false);

document.getElementById('Pause').addEventListener('click',function() {
    send_to.Server("pause");
},false)

document.getElementById('Resume').addEventListener('click',function() {
    send_to.Server("resume");
},false)

// document.getElementById('Callstack').addEventListener('click',function() {
//     send_toServer("callstack");
// },false)

document.getElementById('Reset').addEventListener('click',function() {
    if(currentStatus.collecting) {
        currentStatus.SetWarningMessage("Cannot reset application while collecting data")
        console.log("collecting") 
    }
    else {
        Window.decoder = enc.Decoder();
        console.log("reset") 
    }
},false)

document.getElementById('ShouldVisualize').addEventListener('click', function() {
    console.log("should visualize")
    if(currentStatus.collecting) {
        console.log("status is collecting do nothing")
        currentStatus.SetWarningMessage("Cannot enable visualization during runtime")
        return;
    }
    else {
        currentStatus.visualization_enabled = !status.visualization_enabled;
        if(status.visualization_enabled) {
                send_to.Server('enable-pipe');            
                var canvas = document.createElement('canvas');
                canvas.id     = "myChart";
                canvas.width  = 400;
                canvas.height = 400;
                document.getElementById('chart-goes-here').appendChild(canvas);
                var legend = document.createElement('div');
                legend.className = "chart-legend";
                legend.id = "js-legend";
                document.getElementById('legend-goes-here').appendChild(legend);
                visualization.initChart();
            }
            else {

                send_to.Server('disable-pipe');
                var elem = document.getElementById("myChart");
                elem.remove();
                elem = document.getElementById("js-legend");
                elem.remove();
            }
    }
    
},false)

document.getElementById("byte_conversion").addEventListener('change', function() {
    var node = document.getElementById("byte_conversion");
    visualization.shiftYScale(node.value);
})