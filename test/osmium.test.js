var osmium = require('../');
var assert = require('assert');

describe('osmium', function() {

    it('should be able to create an osmium.Reader', function(done) {
        var file = new osmium.File(__dirname+"/data/berlin-latest.osm.pbf");
        var reader = new osmium.Reader(file, {});
        var header = reader.header();
        assert.equal(header.generator, 'Osmium (http://wiki.openstreetmap.org/wiki/Osmium)');
        var bounds = header.bounds;
        var expected = [ 13.08283, 52.33446, 13.76136, 52.6783 ];
        assert.ok(Math.abs(bounds[0] - expected[0]) < .000000001);
        assert.ok(Math.abs(bounds[1] - expected[1]) < .000000001);
        assert.ok(Math.abs(bounds[2] - expected[2]) < .000000001);
        assert.ok(Math.abs(bounds[3] - expected[3]) < .000000001);
        reader.close();
        done();
    });

    it('should be able to apply a handler to a reader', function(done) {
        var handler = new osmium.Handler();
        var nodes = 0;
        handler.on('node',function(node) {
            ++nodes;
        });
        handler.on('done',function() {
            assert.equal(nodes >= 1993505, true);
        });
        var file = new osmium.File(__dirname+"/data/berlin-latest.osm.pbf");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        // since reader.apply is sync, we can re-use handlers
        var file2 = new osmium.File(__dirname+"/data/winthrop.osm");
        var reader2 = new osmium.Reader(file2);
        nodes = 0;
        handler.on('done',function() {
            assert.equal(nodes, 1525);
            done();
        });
        reader2.apply(handler);
    });

    it('should be able to get node data from handler parameter', function(done) {
        var handler = new osmium.Handler();
        var nodes = 0, ways = 0;
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
        handler.on('done', function() {
            assert.equal(nodes, 1525);
            done();
        });
        var reader = new osmium.Reader(__dirname+"/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(handler);
    });

});
