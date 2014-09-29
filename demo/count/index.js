#!/usr/bin/env node

var osmium = require('../../');

if (process.argv.length != 3) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE");
    process.exit(1);
}

var input_filename = process.argv[2];

// =====================================

var handler = new osmium.Handler();

var nodes=0, ways=0, relations=0;

handler.on('node', function(node) {
    nodes++;
});

handler.on('way', function(way) {
    ways++;
});

handler.on('relation', function(relation) {
    relations++;
});

var reader = new osmium.Reader(input_filename);
osmium.apply(reader, handler);
console.log("Nodes: " + nodes);
console.log("Ways: " + ways);
console.log("Relations: " + relations);

