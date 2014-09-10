var osmium = require('./binding/osmium.node');
exports = module.exports = osmium;
exports.version = require('../package').version;

var timestamp_as_date_object = function() {
    return new Date(1000 * this.timestamp_seconds_since_epoch);
}

exports.Node.prototype.timestamp     = timestamp_as_date_object;
exports.Way.prototype.timestamp      = timestamp_as_date_object;
exports.Relation.prototype.timestamp = timestamp_as_date_object;

