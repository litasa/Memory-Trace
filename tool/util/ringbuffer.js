
var RingBuffer = function(size) {
    this.buffer_ = new Buffer(size);

    this.size_ = size;

    this.position_ = 0;

    this.remaining_ = 0;

		this.rollback_ = 0;
};

RingBuffer.prototype.next = function () {
	if(this.remaining_ > 0)
	{
		var value = this.buffer_[this.position_++]
		this.remaining_--;
		this.position_ %= this.size_;
		return value;
	}
	return null;
};

RingBuffer.prototype.add = function (value) {
	if(this.remaining_ < this.size_){
		var index = (this.position_ + this.remaining_) % this.size_;
		this.buffer_[index] = value;
		this.remaining_++;
		return true;
	}
	return false;
}

RingBuffer.prototype.remaining = function() {
	return this.remaining_;
}

RingBuffer.prototype.populate = function(buffer, num_data_left) {
	//fill first to the end of buffer_
	//then fill from start to position_
  var spaceLeft = this.size_ - this.remaining_;
  if (spaceLeft == 0) {
    spaceLeft = this.size_;
  }
	var numToAdd = Math.min(buffer.length, spaceLeft);
  var start = buffer.length - num_data_left;
  if(start != 0) { //so we get correct value
    start -= 1;
  }
  for (var i = start; i < numToAdd; i++) {
		this.add(buffer[i]);
	}
	return buffer.length - numToAdd;
}

RingBuffer.prototype.rollback = function(index) {
	if(index !== undefined) {
		this.setRollback(index);
	}
	var pos = this.position_;
  if(pos < this.rollback_) { //if pos wrapped around
    pos += this.size_;
  }
	var diff = pos - this.rollback_;
	this.position_ = this.rollback_;
	this.remaining_ += diff;
}

RingBuffer.prototype.setRollback = function(index) {
	if (index === undefined) {
		this.rollback_ = this.position_;
	}
	else if(index < this.position_) {
		this.rollback_ = index;
	}
	else {
		throw "Index needs to be lower then current read position for rolling back"
	}
}

module.exports = RingBuffer;
