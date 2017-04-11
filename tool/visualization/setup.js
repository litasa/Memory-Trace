var enc = require('..\\build\\Release\\Encryption');
var Visualization = require("./visualization.js")
var status = require("./status.js")
var send_to = require("../util/send_to.js")

Window.decoder = enc.Decoder();
Window.visualization_enabled = true;

window.onload = function() {
  Visualization.initChart();

  send_to.Server('internal-server-address', {addr: list});
  setInterval(Visualization.setUpdate,500, Visualization);
}