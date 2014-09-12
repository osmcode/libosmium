var osmium = require('../');
var assert = require('assert');

describe('coordinates', function() {

    it('should be able to instantiate coordinates', function(done) {
        var c = new osmium.Coordinates(5.3, 2.1);
        assert.equal(c.lon, 5.3);
        assert.equal(c.lat, 2.1);
        assert.ok(c.valid());
        done();
    });

    it('should use undefined defaults', function(done) {
        var c = new osmium.Coordinates();
        assert.equal(c.lon, undefined);
        assert.equal(c.lat, undefined);
        assert.ok(!c.valid());
        done();
    });

    it('should be invalid for out of bounds', function(done) {
        var c = new osmium.Coordinates(200, 10);
        assert.equal(c.lon, 200);
        assert.equal(c.lat, 10);
        assert.ok(!c.valid());
        done();
    });

    it('box can be default instantiated', function(done) {
        var box = new osmium.Box();

        assert.ok(box);
        assert.equal(box.left(), undefined);

        done();
    });

    it('box can be instantiated with coordinates', function(done) {
        var c1 = new osmium.Coordinates(1.2, 3.4);
        var c2 = new osmium.Coordinates(5.6, 7.8);
        var box = new osmium.Box(c1, c2);

        assert.ok(box);
        assert.equal(box.left(), 1.2);

        done();
    });

});

