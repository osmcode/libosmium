var osmium = require('../');
var assert = require('assert');

describe('osm object creation', function() {

    it('should not be able to create an osmium.OSMObject from Javascript', function() {
        var got_exception = false;
        try {
            var object = new osmium.OSMObject;
        } catch (e) {
            assert.ok(e instanceof TypeError);
            assert.equal(e.message, 'osmium.OSMObject cannot be created in Javascript');
            got_exception = true;
        } finally {
            assert.ok(got_exception);
        }
    });

    it('should not be able to create an osmium.Node from Javascript', function() {
        var got_exception = false;
        try {
            var node = new osmium.Node;
        } catch (e) {
            assert.ok(e instanceof TypeError);
            assert.equal(e.message, 'osmium.Node cannot be created in Javascript');
            got_exception = true;
        } finally {
            assert.ok(got_exception);
        }
    });

    it('should not be able to create an osmium.Way from Javascript', function() {
        var got_exception = false;
        try {
            var way = new osmium.Way;
        } catch (e) {
            assert.ok(e instanceof TypeError);
            assert.equal(e.message, 'osmium.Way cannot be created in Javascript');
            got_exception = true;
        } finally {
            assert.ok(got_exception);
        }
    });

    it('should not be able to create an osmium.Relation from Javascript', function() {
        var got_exception = false;
        try {
            var relation = new osmium.Relation;
        } catch (e) {
            assert.ok(e instanceof TypeError);
            assert.equal(e.message, 'osmium.Relation cannot be created in Javascript');
            got_exception = true;
        } finally {
            assert.ok(got_exception);
        }
    });

});

