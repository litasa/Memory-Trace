var TimeConverter = function() {
    this.tick_time_ = 0;
}

TimeConverter.prototype.setFrequency = function(frequency) {
    var one = new BigNumber(1);
    this.tick_time_ = one.div(frequency);
}

TimeConverter.prototype.getSeconds = function(bignum) {
    return bignum.times(this.tick_time_).toNumber();
}

module.exports = TimeConverter;