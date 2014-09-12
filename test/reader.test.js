var osmium = require('../');
var assert = require('assert');

describe('reader', function() {

    it('should throw when Reader called as function', function(done) {
        assert.throws(function() {
            var reader = osmium.Reader("foo");
        }, Error);
        done();
    });

    it('should not hang when apply() is called twice on reader', function(done) {
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        var handler = new osmium.Handler(); 
        reader.apply(handler);

        assert.throws(function() {
            reader.apply(handler);
        }, Error);
        done();
    });

    it('should be able to create an osmium.Reader and access header', function(done) {
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {});
        var header = reader.header();
        assert.equal(header.generator, 'CGImap 0.2.0');

        var bounds = header.bounds[0];
        assert.ok(Math.abs(bounds.left()   - (-120.2024)) < .000000001);
        assert.ok(Math.abs(bounds.bottom() - (  48.4636)) < .000000001);
        assert.ok(Math.abs(bounds.right()  - (-120.1569)) < .000000001);
        assert.ok(Math.abs(bounds.top()    - (  48.4869)) < .000000001);
        reader.close();

        done();
    });

});

