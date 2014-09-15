var osmium = require('../');
var assert = require('assert');

describe('geojson', function() {

   it('should be able to create geojson from a node', function(done) {
        var handler = new osmium.Handler();
        var count = 0;
        handler.on('node', function(node) {
            if (count == 0) {
                assert.deepEqual(node.geojson(), {
                    type: 'Point',
                    coordinates: [-120.1891610, 48.4655800]
                });
                done();
            }
            count++;
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file, {node: true});
        reader.apply(handler);
    });

   it('should be able to create geojson from a way', function(done) {
        var handler = new osmium.Handler();
        handler.on('way', function(way) {
            if (way.id == 6089456) {
                assert.deepEqual(way.geojson(), {
                    type: 'LineString',
                    coordinates: [
                        [-120.1796227, 48.4798110],
                        [-120.1787663, 48.4802976]
                    ]
                });
                done();
            }
        });
        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(new osmium.LocationHandler(), handler);
    });

});

