
var exports = module.exports = {};

exports.SetMessage = function(message) {
    var node = document.getElementById('status');
    if(node.firstChild ){
    node.removeChild(node.firstChild);
    }
    var msg = document.createTextNode(message)
    node.appendChild(msg);
}

exports.SetIpAddress = function(ip, port) {
    var msg = document.createTextNode(ip + ' : ' + port)
    document.getElementById("connect-message").appendChild(msg);
}

exports.SetWarningMessage = function(message) {
    var node = document.getElementById('status');
    var last_msg = node.firstChild;
    var last_style = node.style.backgroundColor;
    node.removeChild(node.firstChild);
    var msg = document.createTextNode(message);
    node.style.backgroundColor = "red"
    node.appendChild(msg);
    setTimeout(function() {
        node.removeChild(msg);
        node.style.backgroundColor = last_style;
        node.appendChild(last_msg);
    },3000)
}

exports.collecting = false;

exports.visualization_enabled = true;