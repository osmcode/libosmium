#!/usr/bin/env node

var osmium = require('../../');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE DBFILE");
    process.exit(1);
}

var input_file = process.argv[2];
var output_file = process.argv[3];

var converter = new osmium.Converter({ output: output_file, show_layers: true });

converter.add_layer('natural_pois').
    of_type('point').
    with_attribute('osm_id', 'string').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.add_layer('roads').
    of_type('linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string').
    with_attribute('ref', 'string').
    with_attribute('oneway', 'integer').
    with_attribute('maxspeed', 'integer');

converter.add_layer('cycleways').
    of_type('linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('name', 'string');

converter.add_layer('railways').
    of_type('linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('name', 'string');

converter.add_layer('waterways').
    of_type('linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.add_layer('boundaries').
    of_type('multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('level', 'integer').
    with_attribute('name', 'string');

converter.add_layer('landuse').
    of_type('multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.add_layer('water').
    of_type('multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

// ---- rules ----

converter.node('natural', 'tree|peak|spring').
    to_layer('natural_pois').
        attr('type', 'natural').
        attr('name');

converter.way('waterway', 'stream|river|ditch|canal|drain').
    to_layer('waterways').
        attr('type', 'waterway').
        attr('name');

converter.way('highway', /^(motorway|trunk|primary|secondary)(_link)?$/).
    to_layer('roads').
        attr('type', 'highway').
        attr('ref').
        attr('name').
        attr('oneway', function(tags) {
            var o = tags['oneway'];
            if (o == 'yes' || o == 'true' || o == '1') {
                return 1;
            } else if (o == "-1") {
                return -1;
            } else {
                return 0;
            }
        }).
        attr('maxspeed')

converter.way('highway', 'cycleway').
    to_layer('cycleways').
        attr('name');

converter.way('railway', 'rail').
    to_layer('railways').
        attr('name');

converter.area('boundary', 'administrative').
    to_layer('boundaries').
        attr('level', 'admin_level').
        attr('name');

converter.area('landuse', 'forest|grass|residential|farm|meadow|farmland|industrial|farmyard|cemetery|commercial|quarry|orchard|vineyard|allotments|retail|construction|recreation_ground|village_green').
    to_layer('landuse').
        attr('type', 'landuse').
        attr('name');

converter.area('natural', 'water').
    to_layer('water').
        attr('type', 'natural').
        attr('name');

converter.convert(input_file);

