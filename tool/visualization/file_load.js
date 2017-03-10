var rem = require('electron').remote;
var dialog = rem.dialog;
var fs = require('fs');

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
                if(server === null) {
                    console.log('No server to connect to')            
                }
                else {
                    Window.decoder.unpackStreamToDisk(data, "hii");
                }
            });
        }
    });
},false);

document.getElementById('Pause').addEventListener('click',function() {
    sendToServer("pause");
    Window.started = false;
},false)

document.getElementById('Resume').addEventListener('click',function() {
    sendToServer("resume");
    Window.started = true;
},false)

document.getElementById('Callstack').addEventListener('click',function() {
    sendToServer("callstack");
},false)

document.getElementById('ShouldVisualize').addEventListener('click', function() {
    Window.visualization_enabled = !Window.visualization_enabled;
    if(Window.visualization_enabled) {
        alert("Visualization is now enabled");
        var canvas = document.createElement('canvas');
        canvas.id     = "myChart";
        canvas.width  = 400;
        canvas.height = 400;
        document.getElementById('chart-goes-here').appendChild(canvas);
        var legend = document.createElement('div');
        legend.className = "chart-legend";
        legend.id = "js-legend";
        document.getElementById('legend-goes-here').appendChild(legend);
        initChart();

    }
    else {
        alert("Visualization is now disabled")
        var elem = document.getElementById("myChart");
        elem.remove();
        elem = document.getElementById("js-legend");
        elem.remove();
    }
},false)