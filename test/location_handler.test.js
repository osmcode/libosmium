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

    it('should be able to use the default location handler', function() {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        osmium.apply(reader, new osmium.LocationHandler(), get_handler());
    });

    it('should be able to use the sparsetable location handler', function() {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        osmium.apply(reader, new osmium.LocationHandler("sparsetable"), get_handler());
    });

    it('should be able to use the stlmap location handler', function() {
        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        osmium.apply(reader, new osmium.LocationHandler("stlmap"), get_handler());
    });

    it('should throw on missing location if ignoreErrors is not set', function() {
        var reader = new osmium.Reader(__dirname + "/data/missing-node.osm", { 'node': true, 'way': true });
        var location_handler = new osmium.LocationHandler();
        var handler = new osmium.Handler();

        assert.throws(function() {
            osmium.apply(reader, location_handler, handler);
        }, Error);
    });

    it('should throw in wkb/wkt/node_coordinates function if ignoreErrors is set', function(done) {
        var reader = new osmium.Reader(__dirname + "/data/missing-node.osm", { 'node': true, 'way': true });
        var location_handler = new osmium.LocationHandler();
        location_handler.ignoreErrors();

        var handler = new osmium.Handler();

        handler.on('way', function(way) {
            assert.equal(way.id, 10);
            assert.throws(function() {
                way.wkb();
            }, Error);
            assert.throws(function() {
                way.wkt();
            }, Error);
            assert.throws(function() {
                way.node_coordinates();
            }, Error);
            done();
        });

        osmium.apply(reader, location_handler, handler);
    });

    it('should return undefined for node_coordinates(n) function if ignoreErrors is set', function(done) {
        var reader = new osmium.Reader(__dirname + "/data/missing-node.osm", { 'node': true, 'way': true });
        var location_handler = new osmium.LocationHandler();
        location_handler.ignoreErrors();

        var handler = new osmium.Handler();

        handler.on('way', function(way) {
            assert.equal(way.id, 10);
            assert.deepEqual(way.node_refs(), [1, 2]);
            assert.deepEqual(way.node_coordinates(0).lon, 1.02);
            assert.deepEqual(way.node_coordinates(0).lat, 2.03);
            assert.deepEqual(way.node_coordinates(1), undefined);
            done();
        });

        osmium.apply(reader, location_handler, handler);
    });

});
