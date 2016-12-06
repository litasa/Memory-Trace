const ipc = require('node-ipc');
const neDB = require('nedb')

db = new neDB({ filename: './database/test.db'});
db.loadDatabase(function(err) {
  if(err) {console.log(err)};
})

ipc.config.id = 'database';
ipc.config.retry = 0;
ipc.config.silent = true;
events = [];
ipc.serveNet(function() {
  ipc.server.on('save-to-db', function(data, socket) {
    console.log('data recieved');
    db.insert(data, function(err) {
      if(err) {console.log(err)};
    })
  })
})

ipc.server.start();
