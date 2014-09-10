var osmium = require('./binding/osmium.node');
exports = module.exports = osmium;
exports.version = require('../package').version;

exports.OSMObject.prototype.timestamp = function() {
    return new Date(1000 * this.timestamp_seconds_since_epoch);
}

exports.Changeset.prototype.created_at = function() {
    return new Date(1000 * this.created_at_seconds_since_epoch);
}

exports.Changeset.prototype.closed_at = function() {
    if (this.closed_at_seconds_since_epoch == 0) {
        return undefined;
    }
    return new Date(1000 * this.closed_at_seconds_since_epoch);
}

