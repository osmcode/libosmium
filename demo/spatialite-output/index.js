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
var child = exec("spatialite " + output_filename + " \"SELECT InitSpatialMetaData('WGS84');\"");
child.on('exit', function(code, signal) {

    var db = new sqlite3.Database(output_filename);
    db.serialize();

    db.spatialite();
    db.run("PRAGMA synchronous = OFF;"); // otherwise it is very slow

    // we don't need this here because of the workaround above
    //db.run("SELECT InitSpatialMetaData('WGS84');");

    db.run("CREATE TABLE amenities (osm_id INT, name TEXT);");
    db.run("SELECT AddGeometryColumn('amenities', 'geom', 4326, 'POINT', 2);");

    db.run("CREATE TABLE highways (osm_id INT, htype TEXT, name TEXT);");
    db.run("SELECT AddGeometryColumn('highways', 'geom', 4326, 'LINESTRING', 2);");

    var add_amenity = db.prepare("INSERT INTO amenities (osm_id, name, geom) VALUES (?, ?, GeomFromWKB(?, 4326))");
    var add_highway = db.prepare("INSERT INTO highways (osm_id, htype, name, geom) VALUES (?, ?, ?, GeomFromWKB(?, 4326))");

    var handler = new osmium.Handler();

    handler.on('node', function(node) {
        if (node.tags('amenity') == 'school') {
            add_amenity.run(node.id, node.tags('name'), node.wkb()); 
        }
    });

    handler.on('way', function(way) {
        if (way.tags('highway')) {
            if (way.wkt) {
                add_highway.run(way.id, way.tags('highway'), way.tags('name'), way.wkb());
            }
        }
    });

    handler.on('done', function() {
        add_highway.finalize();
        add_amenity.finalize();
        db.close();
    });

    var location_handler = new osmium.LocationHandler();
    var reader = new osmium.Reader(input_filename);
    reader.apply(location_handler, handler);
    reader.close();

});

