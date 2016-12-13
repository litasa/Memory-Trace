
initChart = function () {
		Chart.defaults.global.animation.duration = 0;
		var ctx = document.getElementById("myChart");
		lineData = {
			labels: [],
			datasets: [],
		};

		Visualization.chart = new Chart(ctx, {
			type: 'line',
			data: lineData,
			options: {
				responsive: true,
				animation: {
					duration: 0
				},
				tooltips: {
					enabled: true
				},
				scales: {
					xAxes: [{
						type: 'linear',
						position: 'bottom'
					}]
				},
				zoom: {
					enabled: true,
					drag: false,
					mode: 'x',
				},
				pan: {
					enabled: true,
					drag: true,
					mode: 'x'
				}
			}
		});
		count = 0;
		Visualization.chart.update();
}
/*
Required for a chart object
			name: heap.id,
			legendText: global.SeenStrings.get(heap.nameId),
			type: 'line',
			dataPoints: []
*/
Visualization = new function() {
	this.MaxHorizontal = 1000;
	this.Scale = 100;

	this.dataToDisplay = [];
	this.allocatorsMap = new Array();

	this.update = function() {
		    this.chart.update();
	}

	var chartDataUpdate = function (allocator, core, time, size){

		//TODO Make sure that high values also get added
		allocator.usedMemory += size;
		//TODO Make sure that high values also get added
		core.usedMemory += size;

		lineData.datasets[allocator.id].data.push({x: time, y: core.usedMemory});

		if(lineData.datasets[allocator.id].data.length > Visualization.MaxHorizontal){
			lineData.datasets[allocator.id].data.splice(0,1);
		}
	}
}
