
window.onload = function () {

		Visualization.chart = new CanvasJS.Chart("chartContainer",{
			zoomEnabled: true,
			title :{
				text: "Memory Usage"
			},
			data: Visualization.dataToDisplay
		});
		Visualization.chart.render();
		Visualization.updateIntervalFunction = setInterval(function() {Visualization.chart.render();}, 100);
}
/*
Required for a chart object
			name: heap.id,
			legendText: global.SeenStrings.get(heap.nameId),
			type: 'line',
			dataPoints: []
*/
Visualization = new function() {
	this.dataToDisplay = [];
	this.allocatorsMap = new Array();
	
	this.addAllocator = function(alloc) {
		//TODO error checking
		this.allocatorsMap.push(alloc);
		alloc.cores = [];
		alloc.managedSize = 0;
		alloc.usedMemory = 0;
		
		this.dataToDisplay.push({
			name: alloc.id,
			legendText: global.SeenStrings.get(alloc.nameId),
			type: 'line',
			dataPoints: []
		});
	}
	
	this.addCore = function(core) {
		//TODO error checking
		var allocator = getAllocator(core.id).value;
		allocator.cores.push(core);
		core.allocs = [];
		core.usedMemory = 0;
		//TODO Make sure that high values also get added
		allocator.managedSize += core.size.low;
	}

	this.addAllocation = function(alloc) {
		var allocator = getAllocator(alloc.id).value;
		var core = getCore(allocator.cores,alloc.pointer).value;
		
		core.allocs.push(alloc);
		//TODO Make sure that high values also get added
		core.usedMemory += alloc.size.low;
		//TODO Make sure that high values also get added
		allocator.usedMemory += alloc.size.low;
		
		
		//TODO Use high value of timestamp as well
		var time = alloc.head.timestamp.low / global.timerFrequency.low;
		
		//TODO use special container for visualization to be able to show different stuffs
		this.dataToDisplay[alloc.id].dataPoints.push({
			x: time,
			y: alloc.size.low
		})
	}
	
	this.removeAllocation = function(alloc) {
		var allocator = getAllocator(alloc.id).value;
		var core = getCore(allocator.cores,alloc.pointer).value;
		var allocation = getAlloc(core.allocs,alloc.pointer);
		
		//TODO Make sure that high values also get added
		allocator.usedMemory -= allocation.value.size.low;
		//TODO Make sure that high values also get added
		core.usedMemory -= allocation.value.size.low;
		
		var time = alloc.head.timestamp.low / global.timerFrequency.low;
		
		this.dataToDisplay[alloc.id].dataPoints.push({
			x: time,
			y: allocator.usedMemory
		});
		
		core.allocs.splice(allocation.key,1);
	}
	
	var getAlloc = function(allocs, pointer){
		for(var i = 0; i < allocs.length; ++i) {
			if(pointerEquals(allocs[i].pointer,pointer)){
				return {
					key: i,
					value: allocs[i]}
			}
		}
		return null;
	}
	
	var getCore = function(cores,pointer) {
		for(var i = 0; i < cores.length; ++i){
			if(pointerInsideCore(pointer,cores[i].base)){
				return {
					index: i,
					value: cores[i]}
			}
		}
		return null;
	}
	
	var getAllocator = function(id){
		for(var i = 0; i < Visualization.allocatorsMap.length; ++i){
			var allocator = Visualization.allocatorsMap[i];
			if(allocator.id === id){
				return {
					index: i,
					value: allocator}
			}
		}
		return null;
	}
}
