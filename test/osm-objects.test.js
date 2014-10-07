var osmium = require('../');
var assert = require('assert');

describe('basic', function() {

   it('should be able to access basic attributes from node', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count++ == 0) {
                assert.equal(node.id, 50031066);
                assert.equal(node.visible, true);
                assert.equal(node.version, 2);
                assert.equal(node.changeset, 3137735);
                assert.equal(node.user, "woodpeck_fixbot");
                assert.equal(node.uid, 147510);
                assert.equal(node.timestamp_seconds_since_epoch, 1258416656);
                assert.equal(node.timestamp().toISOString(), '2009-11-17T00:10:56.000Z');
                assert.equal(node.coordinates.lon, -120.1891610);
                assert.equal(node.coordinates.lat, 48.4655800);
                assert.equal(node.lon, -120.1891610);
                assert.equal(node.lat, 48.4655800);
                assert.equal(node.coordinates.lon, -120.1891610);
                assert.equal(node.coordinates.lat, 48.4655800);
                assert.equal(node.wkt(), "POINT(-120.189161 48.46558)");
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {node: true});
        osmium.apply(reader, handler);
    });

   it('should be able to access basic attributes from way', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('way', function(way) {
            if (count++ == 0) {
                assert.equal(way.id, 6091729);
                assert.equal(way.visible, true);
                assert.equal(way.version, 1);
                assert.equal(way.changeset, 417421);
                assert.equal(way.user, "DaveHansenTiger");
                assert.equal(way.uid, 7168);
                assert.equal(way.timestamp_seconds_since_epoch, 1189655636);
                assert.equal(way.timestamp().toISOString(), '2007-09-13T03:53:56.000Z');
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {way: true});
        osmium.apply(reader, handler);
    });

   it('should be able to access basic attributes from relation', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('relation', function(relation) {
            if (count++ == 0) {
                assert.equal(relation.id, 237891);
                assert.equal(relation.visible, true);
                assert.equal(relation.version, 2);
                assert.equal(relation.changeset, 15155909);
                assert.equal(relation.user, "Jano John Akim Franke");
                assert.equal(relation.uid, 42191);
                assert.equal(relation.timestamp_seconds_since_epoch, 1361751094);
                assert.equal(relation.timestamp().toISOString(), '2013-02-25T00:11:34.000Z');
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {relation: true});
        osmium.apply(reader, handler);
    });

   it('should be able to handle object without tags', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count++ == 0) {
                assert.deepEqual(node.tags(), {});
                assert.equal(node.tags("foobar"), undefined);
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {node: true});
        osmium.apply(reader, handler);
    });

   it('should be able access tags on object', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('way', function(way) {
            if (count++ == 0) {
                assert.equal(way.tags().name, "National Fish Hatchery Entranc");
                assert.equal(way.tags().foobar, undefined);
                assert.equal(way.tags("highway"), "residential");
                assert.equal(way.tags("foobar"), undefined);
                assert.throws(function() {
                    way.tags({});
                }, TypeError);
                assert.throws(function() {
                    way.tags("foo", "bar");
                }, TypeError);
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {way: true});
        osmium.apply(reader, handler);
    });

   it('should be able access nodes on ways', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('way', function(way) {
            if (count++ == 0) {
                assert.equal(way.nodes_count, 6);
                assert.equal(way.node_refs().length, 6);
                assert.equal(way.node_refs()[0], 50253600);
                assert.equal(way.node_refs()[5], 50253608);
                assert.equal(way.node_refs(0), 50253600);
                assert.equal(way.node_refs(5), 50253608);
                assert.throws(function() {
                    way.node_refs(6);
                }, RangeError);
                assert.throws(function() {
                    way.node_refs("foo");
                }, TypeError);
                assert.throws(function() {
                    way.node_refs(1, "bar");
                }, TypeError);
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {way: true});
        osmium.apply(reader, handler);
    });

   it('should be able access members on relations', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('relation', function(relation) {
            if (count++ == 0) {
                assert.equal(relation.members_count, 5);
                assert.equal(relation.members().length, 5);
                assert.deepEqual(relation.members()[0], {
                    type: 'w',
                    ref: 40512249,
                    role: 'outer'
                });
                assert.deepEqual(relation.members()[4], {
                    type: 'w',
                    ref: 40512263,
                    role: 'inner'
                });
                assert.deepEqual(relation.members(1), relation.members()[1]);
                assert.throws(function() {
                    relation.members(5);
                }, RangeError);
                assert.throws(function() {
                    relation.members("foo");
                }, TypeError);
                assert.throws(function() {
                    relation.members(1, "bar");
                }, TypeError);
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {relation: true});
        osmium.apply(reader, handler);
    });

   it('should be able to handle missing and invalid coordinates', function() {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            count++;
            if (count == 1) {
                assert.equal(node.coordinates.lon, 1.02);
                assert.equal(node.coordinates.lat, 2.03);
                assert.equal(node.coordinates.valid(), true);
            } else if (count == 2) {
                assert.equal(node.coordinates.lon, undefined);
                assert.equal(node.coordinates.lat, undefined);
                assert.equal(node.coordinates.valid(), false);
            } else if (count == 3) {
                assert.equal(node.coordinates.lon, 190);
                assert.equal(node.coordinates.lat, 190);
                assert.equal(node.coordinates.valid(), false);
            }
        });
        var file = new osmium.File(__dirname + "/data/coordinates-problems.osm");
        var reader = new osmium.Reader(file, {node: true});
        osmium.apply(reader, handler);
    });

});
