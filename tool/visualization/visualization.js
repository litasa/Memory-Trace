var _ = require('underscore')
datasets = [];
Visualization_time = 0;
initChart = function () {
		var ctx = document.getElementById("myChart");
		

		Visualization.chart = new Chart(ctx, {
			type: 'line',
			data: {datasets},
			options: {
				legendCallback: function(chart) {
					var text = [];
					text.push('<ul class="' + chart.id + '-legend">');
					for (var i = 0; i < chart.data.datasets.length; i++) {
						text.push('<li><span style="background-color:' + chart.data.datasets[i].backgroundColor + '"></span>');
						if (chart.data.datasets[i].label) {
							if(chart.data.datasets[i].allocatorType) {
								text.push(chart.data.datasets[i].label + " - " + chart.data.datasets[i].allocatorType);
							}
						}
						text.push('</li>');
					}
					text.push('</ul>');

					return text.join('');
				},
				multiTooltipTemplate: "<%= datasetLabel %> - <%= value %>",
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
				responsive: false,
				pan: {
						enabled: true,
						mode: 'xy'
				},
				zoom: {
					enabled: true,
					mode: 'xy',
					limits: {
						max: 10,
						min: 0.5
					}
				}
			 }
		});
		Visualization.chart.update();
}

Visualization = new function() {
	this.MaxHorizontal = 100;
	this.latest_time = 0;
	this.startTimer = function() {
		setInterval(function() {
			Visualization_time++;
		}, 1000);
	}

	this.newDataset = function(arr) {
		var win_size = parseInt(document.getElementById('window_size').value)
		var min_value = Math.max(this.chart.scales['x-axis-0'].end - win_size, 0);
		var heap_id = 1;
		//console.log(Window.decoder.getFilteredMemorySnapshots(win_size, min_value, heap_id));

		/*

		for(var i = 0; i < this.chart.data.datasets.length; ++i) {
			var dataset = this.chart.data.datasets[i];
			var exists = _.findWhere(arr, {label: dataset.label})
			
			if(exists === undefined) {
				//data does not already exist. Handle it below
				continue;
			}
			var index = _.indexOf(arr,exists);
			arr.splice(index,1);

			dataset.data = dataset.data.concat(exists.data);
			var win_size = parseInt(document.getElementById('window_size').value)
			dataset.data = _.filter(dataset.data, function(point) {
				return (point.x > Visualization.chart.scales['x-axis-0'].end - win_size);
			})
		}
		for(var i = 0; i < arr.length; ++i) {
			var dataset = {
				label: arr[i].label,
				allocatorType: arr[i].type,
				spanGaps: true,
				borderWidth: 1,
				showLines: true,
				pointRadius: 0,
				lineTension: 0,
				data: arr[i].data
			}
			this.chart.data.datasets.push(dataset);
		}
		*/
		this.updateScales();
		this.chart.update();
	}

	this.calcTimeDiff = function(dataset) {
		return _.last(dataset.data).x - _.first(dataset.data).x;
	}

	this.removeDatasets = function(arr) {
		for(var i = 0; i < arr.length; ++i) {
			var exists = _.findWhere(this.chart.data.datasets, {label: arr[i]})
			if(exists === undefined) {
				continue;
			}
			var index = _.indexOf(this.chart.data.datasets, exists);
			this.chart.data.datasets.splice(index,1);
		}
	}

	this.updateScales = function() {
		if(document.getElementById('follow_alloc').checked) {
			var win_size = parseInt(document.getElementById('window_size').value)
			if(isNaN(win_size)) {
				win_size = 5;
			}
			this.chart.scales['x-axis-0'].options.ticks.min = Math.max(this.chart.scales['x-axis-0'].end - win_size, 0);
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
	Visualization.updateScales();
	document.getElementById('js-legend').innerHTML = Visualization.chart.generateLegend();
	Visualization.chart.update();
}, 500);