var osmium = require('../');
var assert = require('assert');

describe('file', function() {

    it('should throw when File called as function', function() {
        assert.throws(function() {
            var file = osmium.File();
        }, Error);
    });

    it('should create File object without parameters', function() {
        assert.throws(function() {
            var file = new osmium.File();
        }, Error);
    });

    it('should create File object with one parameters', function() {
        var file = new osmium.File("file.osm");
    });

    it('should create File object with two parameters', function() {
        var file = new osmium.File("file.osm", "osm");
    });

    it('should create File object for stdin/stdout', function() {
        var file = new osmium.File("", "osm");
        file = new osmium.File("-", "osm");
    });

    it('should not create File object with three parameters', function() {
        assert.throws(function() {
            var file = new osmium.File("file.osm", "osm", "foo");
        }, TypeError);
    });

});

