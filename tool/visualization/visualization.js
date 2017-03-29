var _ = require('underscore')
var status = require("./status.js")

var exports = module.exports = {};
var datasets = [];


exports.initChart = function () {
	var ctx = document.getElementById("myChart");
	exports.chart = new Chart(ctx, {
		type: 'line',
		data: {datasets},
		options: {
			legendCallback: function(chart) {
				var text = [];
				text.push('<ul class="' + chart.id + '-legend">');
				for (var i = 0; i < chart.data.datasets.length; i++) {
					text.push('<li><span style="background-color:' + chart.data.datasets[i].htmlColor + '">');
					if (chart.data.datasets[i].label) {
						if(chart.data.datasets[i].allocatorType) {
							text.push(chart.data.datasets[i].label + " - " + chart.data.datasets[i].allocatorType);
						}
					}
					text.push('</span></li>');
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
				enabled: true,
			},
			animation: {
				duration: 0
			},
			responsive: true,
			maintainAspectRatio: false,
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
	exports.chart.update();
	status.SetMessage("Waiting for connection")
}

exports.setUpdate = function(visualization) {
	if(status.visualization_enabled) {
		var win_size = visualization.chart.scales['x-axis-0'].max - visualization.chart.scales['x-axis-0'].min
		var min_value = Math.max(visualization.chart.scales['x-axis-0'].end - win_size, 0);
		var heap_id = parseInt(document.getElementById('heap_id').value);
		if(isNaN(heap_id)) {
			heap_id = 0;
		}
		var max_samples_per_second = parseInt(document.getElementById('samples_per_second').value);
		if(isNaN(max_samples_per_second)) {
			max_samples_per_second = 1;
		}
		var byte_conversion = parseInt(document.getElementById("byte_conversion").value);
		visualization.chart.data.datasets = Window.decoder.getFilteredMemorySnapshots(win_size, min_value, heap_id, max_samples_per_second, byte_conversion);
		if(visualization.chart.data.datasets !== false) {
			Window.current_time = visualization.chart.data.datasets[2];
			visualization.chart.data.datasets.splice(2,1);
		}
		
		if(document.getElementById('follow_alloc').checked && status.collecting === true) {
			var wanted_win_size = parseInt(document.getElementById('max_view_window').value);
			var win_size = Math.min(visualization.chart.scales['x-axis-0'].max - visualization.chart.scales['x-axis-0'].min, wanted_win_size);
			visualization.chart.scales['x-axis-0'].options.ticks.max = Math.max(Window.current_time, 1);			
			visualization.chart.scales['x-axis-0'].options.ticks.min = Math.max(visualization.chart.scales['x-axis-0'].end - win_size, 0);
		}
		document.getElementById('js-legend').innerHTML = visualization.chart.generateLegend();
		visualization.chart.update();
	}
}

exports.shiftYScale = function(shift) {
	var current_val = visualization.chart.scales['y-axis-0'].max
	var maxval = current_val;
	console.log(maxval + "before")
	for(var i = 0; i < Math.abs(shift); ++i) {
		if(shift > 0) {
			maxval = maxval >> 10;
		}
		else {
			maxval = maxval << 10;
		}
	}
	if(maxval === 0) {
		maxval = 0.1;
	}
	console.log(maxval + "after")
	visualization.chart.scales['y-axis-0'].options.ticks.suggestedMax = maxval;
}