class CircularBuffer{
	constructor(size) {
		this.data = Buffer.alloc(size);
		this.end = 0;
		this.read = 0;
		this.length = this.data.length;
		this.numRead = 0;
		this.full = false;
		this.readWrap = false;
		this.rollback = 0;
	}
	
	addValue(value) {
		this.data[this.end++] = value;
		if(this.end >= this.length) {
			this.end = 0;
			this.full = true;
		}
	}
	
	getNextValue() {
		var val = this.data[this.read++];
		this.numRead++;
		if(this.read == this.length) {
			this.read = 0;
			this.readWrap = true;
		}
		return val;
	}
	
	readOverflow(length) {
		if(!this.full){
			return this.read >= this.end;
		}
		else {
			if(this.readWrap) {
				return this.read >= this.end;
			}
			return false;
		}
	}
	
	populate(buffer, fromIndex) {
		var temp = this.end;
		var i = fromIndex;
		do{
			this.addValue(buffer[i++]);
		} while(i < buffer.length && temp != this.end);
		return i; //number of items added
	}
	
	resetRead() {
		this.numRead = 0;
		this.readWrap = false;
	}
}