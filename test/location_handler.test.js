var osmium = require('../');
var assert = require('assert');

describe('location handler', function() {

    it('should be able to use location handler', function(done) {
        var location_handler = new osmium.LocationHandler();
        var handler = new osmium.Handler();

        var ways = 0;
        handler.on('way', function(way) {
            if (ways == 0) {
                assert.equal(way.wkt(), "LINESTRING(-120.1872774 48.4715898,-120.188291 48.472511,-120.188374 48.472591,-120.188496 48.472707,-120.188625 48.47283,-120.18914 48.473561)");
            }
            ++ways;
        });

        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(location_handler, handler);

        done();
    });

});
