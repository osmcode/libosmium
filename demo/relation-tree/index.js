#!/usr/bin/env node

var osmium = require('../../');
var buffered_writer = require('buffered-writer');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE OUTFILE");
    process.exit(1);
}

var input_filename = process.argv[2];
var output_filename = process.argv[3];

var stream = buffered_writer.open(output_filename);

// =====================================

var handler = new osmium.Handler();

var relations = {};

handler.on('relation', function(relation) {
    relations[relation.id] = [ relation.tags('type') || '(undefined)', relation.members().filter(function(member) {
        return member.type == 'r';
    }).map(function(member) {
        return member.ref;
    })];
});

function indent(n) {
    return '                                                  '.substr(0, n*2);
}

function print_recurse(level, id) {
    stream.write(indent(level) + id);

    if (relations[id]) {
        stream.write(' type=' + relations[id][0] + "\n");
        if (relations[id].length > 2) {
            if (relations[id][1].length > 0) {
                stream.write(indent(level+1) + "LOOP!\n");
            }
        } else {
            relations[id][1].forEach(function(child) {
                print_recurse(level+1, child);
            });
            relations[id][2] = 'done';
        }
    } else {
        stream.write(" (incomplete)\n");
    }
}

handler.on('done', function() {
    var ids = Object.keys(relations).sort(function(a, b) {
        return a - b;
    });
    ids.forEach(function(id) {
        if (relations[id].length == 2) {
            print_recurse(0, id);
        }
    });
    stream.close();
});

var reader = new osmium.Reader(input_filename);
reader.apply(handler);

