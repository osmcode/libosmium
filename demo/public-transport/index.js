#!/usr/bin/env node

var osmium = require('../../');
var fs = require('fs');
var geojsonStream = require('geojson-stream');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE DBFILE");
    process.exit(1);
}

var input_filename = process.argv[2];
var output_filename = process.argv[3];

var route_relations = {},
    ways = {};

// FIRST PASS

var handler = new osmium.Handler();

handler.on('relation', function(relation) {
    if (relation.tags('type') == 'route_master') {
        var route_info = {
            ref: relation.tags('ref'),
            colour: relation.tags('colour'),
            rtype: relation.tags('route_master')
        };
        relation.members().forEach(function(member) {
            if (member.type == 'r') {
                route_relations[member.ref] = route_info;
            }
        });
    }
});

var reader = new osmium.Reader(input_filename);
osmium.apply(reader, handler);
console.log("first pass done");

// SECOND PASS

handler.on('relation', function(relation) {
    var info = route_relations[relation.id];
    if (info) {
        relation.members().forEach(function(member) {
            if (member.type == 'w') {
                ways[member.ref] = info;
            }
        });
    }
});

reader = new osmium.Reader(input_filename);
osmium.apply(reader, handler);
console.log("second pass done");

// THIRD PASS

handler = new osmium.Handler();

handler.on('way', function(way) {
    var info = ways[way.id];
    if (info) {
        geojsonOut.write({
            type: 'Feature',
            geometry: way.geojson(),
            properties: {
                way_id: way.id,
                ref:    info.ref,
                colour: info.colour,
                rtype:  info.rtype
            }
        });
    }
});

var fileOut = fs.createWriteStream(output_filename);
var geojsonOut = geojsonStream.stringify();
geojsonOut.pipe(fileOut);

var location_handler = new osmium.LocationHandler();

reader = new osmium.Reader(input_filename);
osmium.apply(reader, location_handler, handler);
geojsonOut.end();
console.log("third pass done");

