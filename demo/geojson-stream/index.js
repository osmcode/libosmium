#!/usr/bin/env node

if (!process.argv[2]) {
    console.log('Usage: \n');
    console.log('Write to file:\n\t./index.js --input file.osm --output file.geojson');
    console.log('Write to stdout:\n\t./index.js --input file.osm --output stdout');
    process.exit(1);
}
var osmium = require('../../'),
    fs = require('fs'),
    geojsonStream = require('geojson-stream');

var argv = require('minimist')(process.argv, {
    string: 'input',
    string: 'output',
});

function stop(msg) {
    console.error(msg);
    process.exit(-1);
}

if (!argv.output || typeof(argv.output) != 'string') stop('--output argument required (path to a new file or the keyword "stdout")');
if (!argv.input || typeof(argv.input) != 'string') stop('--input argument required (path to an osm file)');

var reader = new osmium.Reader(argv.input);

var geojsonOut = geojsonStream.stringify();

if (argv.output === 'stdout') {
    geojsonOut.pipe(process.stdout);
} else {
    var fileOut = fs.createWriteStream(argv.output);
    geojsonOut.pipe(fileOut);
}

var handler = new osmium.Handler(); 
handler.on('node',function(node) {
    geojsonOut.write({
        type: 'Feature',
        geometry: node.geojson(),
        properties: {}
    });
});
 
handler.on('done',function() {
    geojsonOut.end();
});
 
reader.apply(handler);
