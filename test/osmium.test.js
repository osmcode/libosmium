var osmium = require('../');
var assert = require('assert');

describe('osmium', function() {

    it('should be able to get node data from handler parameter', function(done) {
        var handler = new osmium.Handler();
        var nodes = 0, ways = 0, relations = 0;
        handler.on('node', function(node) {
            if (nodes == 0) {
                assert.equal(node.id, 50031066);
                assert.equal(node.lon, -120.1891610);
            }
            if (nodes == 1) {
                assert.equal(node.id, 50031085);
                assert.equal(node.lon, -120.1929190);
            }
            if (node.id == 1564464078) {
                assert.equal(node.changeset, 10220832);
                assert.equal(node.tags().amenity, 'pub');
                assert.equal(node.tags('name'), 'Old Schoolhouse Brewery');
            }
            ++nodes;
        });
        handler.on('way', function(way) {
            if (ways == 0) {
                assert.equal(way.id, 6091729);
                assert.equal(way.nodes(1), 50253602);
                assert.deepEqual(way.nodes(), [50253600, 50253602, 50137292, 50137371, 50253605, 50253608]);
            }
            ++ways;
        });
        handler.on('relation', function(relation) {
            if (relations == 0) {
                assert.equal(relation.id, 237891);
                assert.deepEqual(relation.members()[0], ['w', 40512249, 'outer']);
                assert.deepEqual(relation.members(3), ['w', 40512257, 'inner']);
            }
            ++relations;
        });
        handler.on('done', function() {
            assert.equal(nodes, 1525);
            done();
        });
        var reader = new osmium.Reader(__dirname+"/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(handler);
    });

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
        handler.on('done', function() {
            done();
        });
        var reader = new osmium.Reader(__dirname+"/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(location_handler, handler);
    });

    it('should be able to call before and after callbacks', function(done) {
        var handler = new osmium.Handler();
        var nodes = 0;
        var before_nodes = 0, after_nodes = 0;

        handler.on('init', function() {
            assert.equal(nodes, 0);
        });
        handler.on('before_nodes', function() {
            assert.equal(nodes, 0);
            before_nodes++;
        });
        handler.on('node', function(node) {
            ++nodes;
        });
        handler.on('after_nodes', function() {
            assert.equal(nodes >= 1500, true);
            after_nodes++;
        });
        handler.on('done', function() {
            assert.equal(nodes >= 1500, true);
            done();
        });

        var file = new osmium.File(__dirname+"/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(before_nodes, 1);
        assert.equal(after_nodes, 1);
    });

    it('should be able to call two handlers one after the other', function(done) {
        var handler1 = new osmium.Handler();
        var handler2 = new osmium.Handler();

        var count=0;
        handler1.on('init', function() {
            assert.equal(count, 0);
            count++;
        });
        handler2.on('init', function() {
            assert.equal(count, 1);
            count++;
        });

        var file = new osmium.File(__dirname+"/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler1, handler2);

        assert.equal(count, 2);
        done();
    });

});
