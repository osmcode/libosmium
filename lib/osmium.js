var osmium = require('./binding/osmium.node');
exports = module.exports = osmium;
exports.version = require('../package').version;

exports.OSMObject.prototype.timestamp = function() {
    return new Date(1000 * this.timestamp_seconds_since_epoch);
}


