var osmium = require('../');
var assert = require('assert');

describe('basic', function() {

   it('should be able to access basic attributes from node', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node',function(node) {
            if (count == 0) {
                assert.equal(node.id, 50031066);
                assert.equal(node.visible, true);
                assert.equal(node.version, 2);
                assert.equal(node.changeset, 3137735);
                assert.equal(node.user, "woodpeck_fixbot");
                assert.equal(node.uid, 147510);
                assert.equal(node.timestamp_seconds_since_epoch, 1258416656);
                assert.equal(node.timestamp().toISOString(), '2009-11-17T00:10:56.000Z');
                assert.equal(node.lon, -120.1891610);
                assert.equal(node.lat, 48.4655800);
                done();
            }
            count++;
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {node: true});
        reader.apply(handler);
    });

   it('should be able to access basic attributes from way', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('way',function(way) {
            if (count == 0) {
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
            count++;
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {way: true});
        reader.apply(handler);
    });

   it('should be able to access basic attributes from relation', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('relation',function(relation) {
            if (count == 0) {
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
            count++;
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {relation: true});
        reader.apply(handler);
    });

});
