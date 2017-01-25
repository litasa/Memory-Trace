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
				zoom: {
					enabled: true,
					mode: 'xy'
				},
				responsive: false,
			 }
		});
		count = 0;
		Visualization.chart.update();
}

Visualization = new function() {
	this.MaxHorizontal = 100;
	this.Scale = 100;
	this.updated = true;

	this.newDataset = function(arr) {
		for(var i = 0; i < this.chart.data.datasets.length; ++i) {
			var exists = _.findWhere(arr, {label: this.chart.data.datasets[i].label})
			
			if(exists === undefined) {
				continue;
			}
			var index = _.indexOf(arr,exists);
			arr.splice(index,1);

			this.chart.data.datasets[i].data = this.chart.data.datasets[i].data.concat(exists.data);

			// if(this.chart.data.datasets[i].data.length > 300) {
			// 	var items_to_remove = this.chart.data.datasets[i].data.length - 300;
			// 	this.chart.data.datasets[i].data.splice(0,items_to_remove);
			// 	//console.log("current size" + this.chart.data.datasets[i].data.length);
			// }
			//this.chart.data.datasets.pointRadius = 0;
			//this.chart.data.datasets.spawnGaps = true;
		}
		for(var i = 0; i < arr.length; ++i) {
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
	document.getElementById('js-legend').innerHTML = Visualization.chart.generateLegend();	
}, 2000);

