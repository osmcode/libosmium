var osmium = require('../');
var assert = require('assert');

describe('changesets', function() {

   it('should be able to access basic attributes from closed changeset', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('changeset', function(changeset) {
            if (count == 0) {
                assert.equal(changeset.id, 15449957);
                assert.equal(changeset.user, "Elbert");
                assert.equal(changeset.uid, 1237205);
                assert.equal(changeset.num_changes, 10);
                assert.equal(changeset.created_at_seconds_since_epoch, 1363918135);
                assert.equal(changeset.created_at().toISOString(), '2013-03-22T02:08:55.000Z');
                assert.equal(changeset.closed_at_seconds_since_epoch, 1363918138);
                assert.equal(changeset.closed_at().toISOString(), '2013-03-22T02:08:58.000Z');
                assert.equal(changeset.bounds.left(), 120.2988730);
                assert.equal(changeset.bounds.bottom(), -10.0004425);
                assert.equal(changeset.bounds.right(), 120.2991740);
                assert.equal(changeset.bounds.top(), -10.0002384);
                assert.ok(changeset.closed);
                done();
            }
            count++;
        });
        var file = new osmium.File(__dirname + "/data/changesets.osm");
        var reader = new osmium.Reader(file, {changeset: true});
        reader.apply(handler);
    });

   it('should be able to access basic attributes from open changeset', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('changeset', function(changeset) {
            if (count == 2) {
                assert.equal(changeset.id, 15450185);
                assert.equal(changeset.user, "garl");
                assert.equal(changeset.uid, 51196);
                assert.equal(changeset.num_changes, 0);
                assert.equal(changeset.created_at_seconds_since_epoch, 1363926025);
                assert.equal(changeset.created_at().toISOString(), '2013-03-22T04:20:25.000Z');
                assert.equal(changeset.closed_at_seconds_since_epoch, 0);
                assert.equal(changeset.closed_at(), undefined);
                assert.equal(changeset.bounds, undefined);
                assert.ok(changeset.open);
                done();
            }
            count++;
        });
        var file = new osmium.File(__dirname + "/data/changesets.osm");
        var reader = new osmium.Reader(file, {changeset: true});
        reader.apply(handler);
    });

   it('should be able to access tags from changeset', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('changeset', function(changeset) {
            if (count == 0) {
                assert.equal(changeset.tags().created_by, 'JOSM/1.5 (5356 en)');
                assert.equal(changeset.tags('created_by'), 'JOSM/1.5 (5356 en)');
                done();
            }
            count++;
        });
        var file = new osmium.File(__dirname + "/data/changesets.osm");
        var reader = new osmium.Reader(file, {changeset: true});
        reader.apply(handler);
    });

});
