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

exports.Coordinates = function(lon, lat) {
    this.lon = lon;
    this.lat = lat;
    this.valid = function() {
        return this.lon !== undefined &&
               this.lat !== undefined &&
               this.lon >= -180.0 &&
               this.lon <= 180.0 &&
               this.lat >= -90.0 &&
               this.lat <= 90.0;
    };
}

exports.Box = function(bottom_left, top_right) {
    this.bottom_left = bottom_left;
    this.top_right = top_right;

    this.left   = function() { return this.bottom_left === undefined ? undefined : this.bottom_left.lon; };
    this.bottom = function() { return this.bottom_left === undefined ? undefined : this.bottom_left.lat; };
    this.right  = function() { return this.top_right   === undefined ? undefined : this.top_right.lon; };
    this.top    = function() { return this.top_right   === undefined ? undefined : this.top_right.lat; };
}

