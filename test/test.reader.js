var osmium = require('../');
var assert = require('assert');

describe('reader', function() {

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

});

