
datasets = [{data: []}];
initChart = function () {
		var ctx = document.getElementById("myChart");
		

		Visualization.chart = new Chart(ctx, {
			type: 'line',
			data: {datasets},
			options: {
				scales: {
					xAxes: [{
						type: 'linear',
						position: 'bottom'
					}]
        		},
				legend: {
            		display: false,
        		},
				tooltips: {
					enabled: false,
				},
				animation: {
					duration: 0
				},
				zoom: {
					enabled: true,
					mode: 'xy'
				}
			 }
		});
		count = 0;
		Visualization.chart.update();
}

Visualization = new function() {
	this.MaxHorizontal = 100;
	this.Scale = 100;

	this.update = function() {
		this.chart.update();
	}

	this.newDataset = function(arr) {
		for(var i = 0; i < arr.length; ++i) {
			//arr[i].lineTension =  0,
			if(this.chart.data.datasets[i] == undefined) {
				this.chart.data.datasets[i] = {data: []};
			}
			this.chart.data.datasets[i].data = this.chart.data.datasets[i].data.concat(arr[i].data);
			while(this.chart.data.datasets[i].data.length > 5000) {
				this.chart.data.datasets[i].data.shift();
			}
			this.chart.data.datasets[i].borderWidth = 1;
			if(i == 1) {
				this.chart.data.datasets[i].borderColor = 'rgba(250, 250, 1, 1)';
			}
			
			this.chart.data.datasets[i].showLines = true;
			this.chart.data.datasets[i].lineTension = 0;
			//this.chart.data.datasets.pointRadius = 0;
			//this.chart.data.datasets.spawnGaps = true;
		}
	}
}

ipcRenderer.on('allocation', function(event, data) {

})

ipcRenderer.on('heap-created', function(event, data) {

})

ipcRenderer.on('memory', function(event, data) {
  //Visualization.chartDataUpdate(data.array);
})

setInterval(function() {
	Visualization.chart.update();
}, 800);