var CircularBuffer = function (size) {
	var bufferSize = size;
	var buffer = new Array(size);
	
	var end = 0;
	var start = 0;
	
	this.addValue = function(value) {
		buffer[end] = value;
		if(end != bufferSize)
		{
			end++;
		}
		else{
			end = 0;
		}
		if(end == start){
			start++;
		}
	}
	
	this.getValue = function(index) {
		var i = index + start;
		
		if( i >= bufferSize) {
			i -= bufferSize;
		}
		
		return buffer[i];
	}
	
}