var enc = require('..\\build\\Release\\Encryption');
var Visualization = require("./visualization.js")
var status = require("./status.js")
var sendTo = require("../util/sendTo.js")

Window.decoder = enc.Decoder();
Window.visualization_enabled = true;

window.onload = function() {
  Visualization.initChart();
  sendTo.Server('internal-server-address', {addr: list});
  setInterval(Visualization.setUpdate,500, Visualization);
}