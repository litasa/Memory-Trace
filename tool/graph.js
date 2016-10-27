const chartJS = require('./node_modules/chart.js/src/chart.js')
const CHART = document.getElementById("lineChart");

Chart.defaults.global.animation.duration = 0;

var startingData = {
  labels: [1, 2, 3, 4, 5, 6, 7],
  datasets: [
    {
      strokeColor: "rgba(220,220,220,0.2)",
      data: [65,58,23,4,1,3,1]
    }
  ]
},
latestLabel = startingData.labels[6];

let lineChart = new Chart(CHART, {
  type: 'line',
  data: startingData
});

setInterval(function() {
  var currentIndex = lineChart.data.labels[lineChart.data.labels.length - 1];
  lineChart.data.datasets[0].data.push(Math.random()*100);
  lineChart.data.labels.push(++currentIndex);
  if (lineChart.data.labels.length > 20) {
    lineChart.data.labels.splice(0,1);
    lineChart.data.datasets[0].data.splice(0,1);
  }
}, 100)
