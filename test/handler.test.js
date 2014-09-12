var osmium = require('../');
var assert = require('assert');

var get_size = function(obj) {
    var size = 0, key;
    for (key in obj) {
        if (obj.hasOwnProperty(key)) size++;
    }
    return size;
};

describe('handler', function() {

    it('should throw when Handler called as function', function(done) {
        assert.throws(function() {
            var handler = osmium.Handler();
        }, Error);
        done();
    });

    it('should be able to initialize handler', function(done) {
        var handler = new osmium.Handler();
        handler.on('node', function(node) {
        });
        done();
    });

    it('should throw when handler initializion is wrong', function(done) {
        var handler = new osmium.Handler();
        assert.throws(function() {
            handler.on();
        }, TypeError);
        assert.throws(function() {
            handler.on("node");
        }, TypeError);
        assert.throws(function() {
            handler.on("node", "foo");
        }, TypeError);
        assert.throws(function() {
            handler.on("foo", function(node) {
            });
        }, RangeError);
        done();
    });

    it('should call all object callbacks of handler', function(done) {
        var handler = new osmium.Handler();

        var count_nodes = 0;
        var count_ways = 0;
        var count_relations = 0;
        var count_changesets = 0;

        handler.on('node',      function(obj) { count_nodes++;      });
        handler.on('way',       function(obj) { count_ways++;       });
        handler.on('relation',  function(obj) { count_relations++;  });
        handler.on('changeset', function(obj) { count_changesets++; });

        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count_nodes, 1525);
        assert.equal(count_ways, 98);
        assert.equal(count_relations, 2);
        assert.equal(count_changesets, 0);

        file = new osmium.File(__dirname + "/data/changesets.osm");
        reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count_nodes, 1525);
        assert.equal(count_ways, 98);
        assert.equal(count_relations, 2);
        assert.equal(count_changesets, 3);

        done();
    });

    it('should call all before and after callbacks of handler', function(done) {
        var handler = new osmium.Handler();

        var count = 0;

        var callbacks = ['init', 'before_nodes', 'after_nodes', 'before_ways', 'after_ways', 'before_relations', 'after_relations', 'done'];

        callbacks.forEach(function (cb) {
            handler.on(cb, function() { count++; });
        });

        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 8);
        done();
    });

    it('should call before_changesets and after_changesets callbacks of handler', function(done) {
        var handler = new osmium.Handler();

        var count = 0;

        var callbacks = ['init', 'before_changesets', 'after_changesets', 'done'];

        callbacks.forEach(function (cb) {
            handler.on(cb, function() { count++; });
        });

        var file = new osmium.File(__dirname + "/data/changesets.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 4);
        done();
    });

    it('should call node callback only for tagged nodes if tagged_nodes_only is set', function(done) {
        var handler = new osmium.Handler();

        var count = 0;
        handler.on('node', function(node) {
            if (get_size(node.tags()) != 0) {
                count++;
            }
        });

        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 15);

        handler.options({ 'tagged_nodes_only': true });
        handler.on('node', function(node) { count--; });

        file = new osmium.File(__dirname + "/data/winthrop.osm");
        reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 0);

        done();
    });

    it('should if options() is called with wrong arguments', function(done) {
        var handler = new osmium.Handler();

        assert.throws(function() {
            handler.options();
        }, TypeError);

        assert.throws(function() {
            handler.options("foo");
        }, TypeError);

        assert.throws(function() {
            handler.options({}, "b");
        }, TypeError);

        done();
    });

    it('should allow a callback to be redefined', function(done) {
        var handler = new osmium.Handler();

        var count = 0;
        handler.on('node', function(node) {
            count++;
        });

        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 1525);

        count = 0;
        handler.on('node', function(node) { });

        file = new osmium.File(__dirname + "/data/winthrop.osm");
        reader = new osmium.Reader(file);
        reader.apply(handler);

        assert.equal(count, 0);

        done();
    });

    it('should be able to get node data from handler parameter', function(done) {
        var handler = new osmium.Handler();
        var nodes = 0, ways = 0, relations = 0;
        handler.on('node', function(node) {
            if (nodes == 0) {
                assert.equal(node.id, 50031066);
                assert.equal(node.lon, -120.1891610);
            }
            if (nodes == 1) {
                assert.equal(node.id, 50031085);
                assert.equal(node.lon, -120.1929190);
            }
            if (node.id == 1564464078) {
                assert.equal(node.changeset, 10220832);
                assert.equal(node.tags().amenity, 'pub');
                assert.equal(node.tags('name'), 'Old Schoolhouse Brewery');
            }
            ++nodes;
        });
        handler.on('way', function(way) {
            if (ways == 0) {
                assert.equal(way.id, 6091729);
                assert.equal(way.nodes(1), 50253602);
                assert.deepEqual(way.nodes(), [50253600, 50253602, 50137292, 50137371, 50253605, 50253608]);
            }
            ++ways;
        });
        handler.on('relation', function(relation) {
            if (relations == 0) {
                assert.equal(relation.id, 237891);
                assert.deepEqual(relation.members()[0], ['w', 40512249, 'outer']);
                assert.deepEqual(relation.members(3), ['w', 40512257, 'inner']);
            }
            ++relations;
        });
        handler.on('done', function() {
            assert.equal(nodes, 1525);
        });

        var reader = new osmium.Reader(__dirname + "/data/winthrop.osm", { 'node': true, 'way': true });
        reader.apply(handler);
        done();
    });

    it('should be able to call two handlers one after the other', function(done) {
        var handler1 = new osmium.Handler();
        var handler2 = new osmium.Handler();

        var count=0;
        handler1.on('init', function() {
            assert.equal(count, 0);
            count++;
        });
        handler2.on('init', function() {
            assert.equal(count, 1);
            count++;
        });

        var file = new osmium.File(__dirname + "/data/winthrop.osm");
        var reader = new osmium.Reader(file);
        reader.apply(handler1, handler2);

        assert.equal(count, 2);
        done();
    });

});

