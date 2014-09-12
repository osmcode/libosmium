var osmium = require('../');
var assert = require('assert');

function get_handler() {
    var handler = new osmium.Handler();

    var ways = 0;
    handler.on('way', function(way) {
        if (ways == 0) {
            assert.equal(way.wkt(), "LINESTRING(-120.1872774 48.4715898,-120.188291 48.472511,-120.188374 48.472591,-120.188496 48.472707,-120.188625 48.47283,-120.18914 48.473561)");
        }
        ++ways;
    });

    return handler;
}

describe('location handler', function() {

    it('should be able to use the default location handler', function(done) {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(new osmium.LocationHandler(), get_handler());
        done();
    });

    it('should be able to use the sparsetable location handler', function(done) {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(new osmium.LocationHandler("sparsetable"), get_handler());
        done();
    });

    it('should be able to use the stlmap location handler', function(done) {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(new osmium.LocationHandler("stlmap"), get_handler());
        done();
    });

});
