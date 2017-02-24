var _ = require('underscore')
datasets = [];
update_time = 500;
Window.started = false;
Window.current_time = 0;
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
		if(document.getElementById('follow_alloc').checked && Window.started === true) {
			var win_size = parseInt(document.getElementById('window_size').value)
			if(isNaN(win_size)) {
				win_size = 5;
			}
			this.chart.scales['x-axis-0'].options.ticks.max = Math.max(Window.current_time, 1);			
			this.chart.scales['x-axis-0'].options.ticks.min = Math.max(this.chart.scales['x-axis-0'].max - win_size, 0);
		}
	}
}

setInterval(function() {
	Visualization.updateScales();
	document.getElementById('js-legend').innerHTML = Visualization.chart.generateLegend();
	Visualization.chart.update();
}, update_time);