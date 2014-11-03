#!/usr/bin/env node

var osmium = require('../../');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE DBFILE");
    process.exit(1);
}

var input_file = process.argv[2];
var output_file = process.argv[3];

var converter = new osmium.Converter({ output: output_file, show_layers: true });

// ---- layers ----

converter.create_layer('turning_circles').
    with_attribute('osm_id', 'string');

converter.create_layer('traffic_signals').
    with_attribute('osm_id', 'string');

converter.create_layer('major_roads', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string').
    with_attribute('ref', 'string').
    with_attribute('oneway', 'integer').
    with_attribute('maxspeed', 'integer');

converter.create_layer('minor_roads', 'linestring').
    with_attribute('osm_id', 'integer').
    with_attribute('type', 'string').
    with_attribute('name', 'string').
    with_attribute('ref', 'string').
    with_attribute('oneway', 'integer').
    with_attribute('maxspeed', 'integer');

converter.create_layer('plazas', 'multipolygon').
    with_attribute('osm_id', 'integer').
    with_attribute('name', 'string');


// ---- rules ----

converter.add_nodes().
    matching('highway', 'traffic_signals').
    to_layer('traffic_signals');

converter.add_nodes().
    matching('highway', 'turning_circle').
    to_layer('turning_circles');

function convert_oneway(tags) {
    var o = tags['oneway'];
    if (o == 'yes' || o == 'true' || o == '1') {
        return 1;
    } else if (o == "-1") {
        return -1;
    } else {
        return 0;
    }
}

converter.add_ways().
    matching('highway', /^(motorway|trunk|primary|secondary)(_link)?$/).
    to_layer('major_roads').
        with_attribute('type', 'highway').
        with_attribute('ref').
        with_attribute('name').
        with_attribute('oneway', convert_oneway).
        with_attribute('maxspeed')

var minor_roads = 'tertiary|unclassified|residential|living_street|pedestrian|service|track|path|footway|cycleway|road|steps';

converter.add_ways().
    matching('highway', minor_roads).
    to_layer('minor_roads').
        with_attribute('type', 'highway').
        with_attribute('ref').
        with_attribute('name').
        with_attribute('oneway', convert_oneway).
        with_attribute('maxspeed');

converter.add_areas().
    matching('highway', minor_roads).
    matching('area', 'yes').
    to_layer('plazas').
        with_attribute('name');

converter.convert(input_file);

