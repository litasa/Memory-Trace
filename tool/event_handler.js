
window.onload = function () {

		Visualization.chart = new CanvasJS.Chart("chartContainer",{
			zoomEnabled: true,
			title :{
				text: "Memory Usage"
			},
			data: Visualization.data
		});
		Visualization.chart.render();
}
Visualization = new function() {
	this.data = [];
	
	this.addHeap = function(heap) {
		this.data.push({
			name: heap.id,
			legendText: global.SeenStrings.get(heap.nameId),
			type: 'line',
			dataPoints: []
		});
	}

	this.addAllocation = function(allocation) {
		var index = getIndexOf(this.data,"name",allocation.id);
		
		var time = allocation.head.timestamp.low / global.timerFrequency.low;
		
		this.data[index].dataPoints.push({
			x: time,
			y: allocation.size.low
		})
	}
	
	this.addCore = function(core) {
		var index = getIndexOf(this.data,"name",core.head.id);
	}

	var getIndexOf = function(array, key, searchVal){
		for(var i = 0; i < array.length; ++i){
			if(array[i][key] === searchVal) {
				return i;
			}
		}
		return null;
	}
}
