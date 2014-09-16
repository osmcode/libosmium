var osmium = require('../');
var fs = require('fs');
var assert = require('assert');

describe('apply', function() {

    it('should be able to call apply with a Buffer and a handler', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count == 0) {
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
                done();
            }
            count++;
        });

        var buffer = fs.readFileSync(__dirname + "/data/winthrop.osm.ser");

        osmium.apply(buffer, handler);
    });

});

