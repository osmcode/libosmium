#!/usr/bin/env node

var osmium = require('../../');

if (process.argv.length != 3) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE");
    process.exit(1);
}

var input_filename = process.argv[2];

// =====================================

var handler = new osmium.Handler();

var count = 0;
handler.on('way', function(way) {
    if (way.tags('highway')) {
        count++;
    }
});

console.log("Reading input file into memory...\n");
var reader = new osmium.Reader(input_filename);
var buffer = reader.read_all();
reader.close();


var year = 2008;
var endyear = (new Date()).getYear() + 1900;

console.log("Calculating stats for years " + year + " to " + endyear + "\n");

while (year <= endyear) {
    var date = new Date(year, 1, 1);

    var tbuffer = buffer.filter_point_in_time(date);

    osmium.apply(tbuffer, handler);
    tbuffer.clear();
    console.log(year + ": " + count);

    count = 0;
    year++;
}

