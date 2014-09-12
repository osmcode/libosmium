#!/usr/bin/env node

var osmium = require('../../');
var fs = require('fs');
var sqlite3 = require('spatialite');
var exec = require('child_process').exec;

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE DBFILE");
    process.exit(1);
}

var input_filename = process.argv[2];
var output_filename = process.argv[3];

// =====================================

if (fs.existsSync(output_filename)) {
    console.log("Output database file '" + output_filename + "' exists.");
    process.exit(1);
}

/* This is a horrible workaround: We initialize the database
   outside of node using the "spatialite" program. This way we
   get a version of the database that fits the spatialite version
   installed on the system instead of the compiled in version 4 of
   the node-spatialite module. Otherwise other programs on the system
   might not be able to open this database. Ugh. */
//var child = exec("spatialite " + output_filename + " \"SELECT InitSpatialMetaData('WGS84');\"", function(error, stdout, stderr) {
//    console.log("prep done");

    var db = new sqlite3.Database(output_filename);
    db.serialize();

    db.spatialite();
    db.run("PRAGMA synchronous = OFF;"); // otherwise it is very slow

    // we don't need this here because of the workaround above
    db.run("SELECT InitSpatialMetaData('WGS84');");

    db.run("CREATE TABLE routes (way_id INT, rtype TEXT, ref TEXT, colour TEXT);");
    db.run("SELECT AddGeometryColumn('routes', 'geom', 4326, 'LINESTRING', 2);");

    var add_route = db.prepare("INSERT INTO routes (way_id, ref, colour, rtype, geom) VALUES (?, ?, ?, ?, GeomFromWKB(?, 4326))");

    // FIRST PASS

    var handler = new osmium.Handler();

    var route_relations = {};

    handler.on('relation', function(relation) {
        if (relation.tags('type') == 'route_master') {
            var route_info = [ relation.tags('ref'), relation.tags('colour'), relation.tags('route_master') ];
            relation.members().forEach(function(member) {
                if (member.type == 'r') {
//                    console.log("RM", relation.id, member.ref);
                    route_relations[member.ref] = route_info;
                }
            });
        }
    });

    var reader = new osmium.Reader(input_filename);
    reader.apply(handler);
    console.log("first pass done");

    // SECOND PASS

    handler = new osmium.Handler();

    var ways = {};

    handler.on('relation', function(relation) {
        var info = route_relations[relation.id];
        if (info) {
            relation.members().forEach(function(member) {
                if (member.type == 'w') {
                    ways[member.ref] = info;
//                    console.log("RO", member.ref, info);
                }
            });
        }
    });

    reader = new osmium.Reader(input_filename);
    reader.apply(handler);
    console.log("second pass done");

    // THIRD PASS

    handler = new osmium.Handler();

    handler.on('way', function(way) {
        var info = ways[way.id];
        if (info) {
//            console.log("W", way.id, info);
            if (way.wkb()) {
                add_route.run(way.id, info[0], info[1], info[2], way.wkb());
            }
        }
    });

    handler.on('done', function() {
        console.log('done with input');
        add_route.finalize();
        db.close();
    });

    var location_handler = new osmium.LocationHandler();

    reader = new osmium.Reader(input_filename);
    reader.apply(location_handler, handler);
    console.log("third pass done");

//});

