var TimeConverter = function() {
    this.tick_time_ = 0;
}

TimeConverter.prototype.setFrequency = function(frequency) {
    this.tick_time_ = 1/(frequency.readUInt32BE(4));
}

TimeConverter.prototype.getSeconds = function(buffer) {
    if(buffer.readUInt32BE() != 0) {
        console.log("overflow of number. Need to handle this")
    }
    return buffer.readUInt32BE(4)*this.tick_time_;
}

module.exports = TimeConverter;