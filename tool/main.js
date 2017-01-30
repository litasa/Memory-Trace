const electron = require('electron')
//const test = require('./build/Release/Encryption')

const ipc = electron.ipcMain
// Module to control application life.
const app = electron.app
// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.

var testing = false;

let chartWindow
let serverWindow
let internalServerWindow
let testWindow

function createWindow () {
  // Create the browser window.
  //internalServerWindow = new BrowserWindow({width: 800, height: 600, title: "InternalServer"})
  if(testing) {
    testWindow = new BrowserWindow({width: 800, height: 600, title: "Test"})
    testWindow.loadURL(`file://${__dirname}/test.html`)
    testWindow.webContents.openDevTools()
  }
  else {
      chartWindow = new BrowserWindow({width: 800, height: 600, title: "Chart"})
  serverWindow = new BrowserWindow({width: 800, height: 600, title: "Server"})
  //serverWindow.hide();
  //
  // and load the index.html of the app.
  //internalServerWindow.loadURL(`file://${__dirname}/internal-server/internal-server.html`)
  chartWindow.loadURL(`file://${__dirname}/visualization/visualization.html`)
  serverWindow.loadURL(`file://${__dirname}/server/server.html`)
  //


  // Open the DevTools.
  chartWindow.webContents.openDevTools()
  serverWindow.webContents.openDevTools()
  //internalServerWindow.webContents.openDevTools()
   chartWindow.on('closed', function () {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    chartWindow = null
    serverWindow = null
    //internalServerWindow = null
  })
  }
  //
  // Emitted when the window is closed.
 
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function () {
  // On OS X it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', function () {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (chartWindow === null) {
    createWindow()
  }
})

ipc.on('to-chart', function(event, data) {
  chartWindow.webContents.send(data.channel, data);
})

ipc.on('to-server', function(event, data) {
  serverWindow.webContents.send(data.channel, data);
})

// ipc.on('to-internal-server', function(event, data) {
//   internalServerWindow.webContents.send(data.channel, data);
// })

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
