var osmium = require('../');
var assert = require('assert');

describe('buffer', function() {

    it('should allow reading into buffer', function(done) {
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);

        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count++ == 0) {
                assert.equal(node.id, 50031066);
                done();
            }
        });

        var buffer = reader.read();
        osmium.apply(buffer, handler);
    });

    it('should allow reading into buffer in a loop', function(done) {
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);

        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count++ == 0) {
                assert.equal(node.id, 50031066);
                done();
            }
        });

        var buffer;
        while (buffer = reader.read()) {
            osmium.apply(buffer, handler);
        }
    });

    it('should allow iterating over buffer', function() {
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);

        var buffer;
        while (buffer = reader.read()) {
            var object = buffer.next();
            assert.ok(object instanceof osmium.Node);
            assert.equal(object.id, 50031066);

            object = buffer.next();
            assert.ok(object instanceof osmium.Node);
            assert.equal(object.id, 50031085);

            var count=0, ways=0;
            while (object = buffer.next()) {
                if (ways == 0 && object instanceof osmium.Way) {
                    ++ways;
                    assert.equal(object.id, 6091729);
                }
                ++count;
            }
            assert.equal(count, 1525 /*nodes*/ + 98 /*ways*/ + 2 /*relations*/ - 2 /*already read*/);
        }
    });

});

