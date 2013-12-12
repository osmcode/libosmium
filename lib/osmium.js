var osmium = require('./osmium.node');
exports = module.exports = osmium;
exports.version = require('../package').version;

exports.Node.prototype.date = function() {
    return new Date(1000*this.timestamp);
}