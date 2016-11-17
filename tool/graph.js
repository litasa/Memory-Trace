const canvasJS = require('./node_modules/canvasjs/dist/canvasjs.min.js')

//---------------------------CANVASJS---------------------------------------------
var meep = [
	  ];

let chart = new CanvasJS.Chart("chartContainer", {
  data: [
  {
	  type: "line",
	  dataPoints: meep
  }
  ]
});

function addChartData(dataArray, newData){
  dataArray.push(newData);

  if (dataArray.length > 500) {
    dataArray.shift();
  }
}


chart.render();
