#!/usr/bin/env node

var osmium = require('../../');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE DBFILE");
    process.exit(1);
}

var input_file = process.argv[2];
var output_file = process.argv[3];

var converter = new osmium.Converter({ output: output_file, show_layers: true });

converter.create_layer('natural_pois', 'point').
    with_attribute('osm_id', 'string').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.create_layer('roads', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string').
    with_attribute('ref', 'string').
    with_attribute('oneway', 'integer').
    with_attribute('maxspeed', 'integer');

converter.create_layer('cycleways', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('name', 'string');

converter.create_layer('railways', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('name', 'string');

converter.create_layer('waterways', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.create_layer('boundaries', 'multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('level', 'integer').
    with_attribute('name', 'string');

converter.create_layer('landuse', 'multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

converter.create_layer('water', 'multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string');

// ---- rules ----

converter.add_nodes().
    matching('natural', 'tree|peak|spring').
    to_layer('natural_pois').
        with_attribute('type', 'natural').
        with_attribute('name');

converter.add_ways().
    matching('waterway', 'stream|river|ditch|canal|drain').
    to_layer('waterways').
        with_attribute('type', 'waterway').
        with_attribute('name');

converter.add_ways().
    matching('highway', /^(motorway|trunk|primary|secondary)(_link)?$/).
    to_layer('roads').
        with_attribute('type', 'highway').
        with_attribute('ref').
        with_attribute('name').
        with_attribute('oneway', function(tags) {
            var o = tags['oneway'];
            if (o == 'yes' || o == 'true' || o == '1') {
                return 1;
            } else if (o == "-1") {
                return -1;
            } else {
                return 0;
            }
        }).
        with_attribute('maxspeed')

converter.add_ways().
    matching('highway', 'cycleway').
    to_layer('cycleways').
        with_attribute('name');

converter.add_ways().
    matching('railway', 'rail').
    to_layer('railways').
        with_attribute('name');

converter.add_areas().
    matching('boundary', 'administrative').
    to_layer('boundaries').
        with_attribute('level', 'admin_level').
        with_attribute('name');

converter.add_areas().
    matching('landuse', 'forest|grass|residential|farm|meadow|farmland|industrial|farmyard|cemetery|commercial|quarry|orchard|vineyard|allotments|retail|construction|recreation_ground|village_green').
    to_layer('landuse').
        with_attribute('type', 'landuse').
        with_attribute('name');

converter.add_areas().
    matching('natural', 'water').
    to_layer('water').
        with_attribute('type', 'natural').
        with_attribute('name');

converter.convert(input_file);

