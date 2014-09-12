var osmium = require('../');
var assert = require('assert');

describe('file', function() {

    it('should throw when File called as function', function(done) {
        assert.throws(function() {
            var file = osmium.File();
        }, Error);
        done();
    });

    it('should create File object without parameters', function(done) {
        assert.throws(function() {
            var file = new osmium.File();
        }, Error);
        done();
    });

    it('should create File object with one parameters', function(done) {
        var file = new osmium.File("file.osm");
        done();
    });

    it('should create File object with two parameters', function(done) {
        var file = new osmium.File("file.osm", "osm");
        done();
    });

    it('should create File object for stdin/stdout', function(done) {
        var file = new osmium.File("", "osm");
        file = new osmium.File("-", "osm");
        done();
    });

    it('should not create File object with three parameters', function(done) {
        assert.throws(function() {
            var file = new osmium.File("file.osm", "osm", "foo");
        }, TypeError);
        done();
    });

});

