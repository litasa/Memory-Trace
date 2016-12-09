const neDB = require('nedb')
var date = new Date();
var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
var name =  "./database/test_" + time + ".db";
db = new neDB({ filename: name});
db.loadDatabase(function(err) {
  if(err) {console.log(err)};
})

  db.insert(m, function(err, newdoc) {
    if(err) {console.log(err)}
  });
