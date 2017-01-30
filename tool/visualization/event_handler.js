var _ = require('underscore')
datasets = [];
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
					enabled: true,
				},
				animation: {
					duration: 0
				},
				responsive: false,
			 }
		});
		Visualization.chart.update();
}

Visualization = new function() {
	this.MaxHorizontal = 100;
	this.latest_time = 0;
	this.newDataset = function(arr) {
		for(var i = 0; i < this.chart.data.datasets.length; ++i) {
			var exists = _.findWhere(arr, {label: this.chart.data.datasets[i].label})
			
			if(exists === undefined) {
				//data does not already exist. Handle it below
				continue;
			}
			var index = _.indexOf(arr,exists);
			arr.splice(index,1);

			this.chart.data.datasets[i].data = this.chart.data.datasets[i].data.concat(exists.data);
			if(this.chart.data.datasets[i].data.length == 0) {
				this.chart.data.datasets.splice(i,1);
				console.log("removing items")
			}
		}
		for(var i = 0; i < arr.length; ++i) {
			if(arr[i].data.length == 0) {
				console.log("removing items 2")
				continue;
			}
			var dataset = {
				label: arr[i].label,
				allocatorType: arr[i].type,
				spanGaps: true,
				borderWidth: 1,
				showLines: true,
				lineTension: 0,
				data: arr[i].data
			}
			this.chart.data.datasets.push(dataset);
		}
		Visualization.chart.update();
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
	//console.log(Visualization.chart.options.scales.xAxes[0].ticks.min);
	Visualization.chart.update();
	document.getElementById('js-legend').innerHTML = Visualization.chart.generateLegend();	
}, 2000);

