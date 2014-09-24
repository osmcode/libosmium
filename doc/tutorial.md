
# Osmium Node Module Tutorial

## Using the node-osmium Library

Install with

    npm install osmium

and add

    var osmium = require('osmium');

in your Javascript file to access it.

## OSM File Access

`node-osmium` can read all the popular types of OSM files. It can read the XML
format (without compression (`.osm`) or with gzip (`.osm.gz`) or bzip2
(`.osm.bz2`) compression), the PBF format (`.osm.pbf`), files with (`.osh`) or
without (`.osm`) historic data and change files (`.osc`). It can also read OSM
files with changesets.

To open such a file you need a `File` object:

    var file = new osmium.File("some_file.osm");

Osmium will detect the file format from the filename suffix. It understands all
the usual suffixes. If it doesn't understand the suffix, you can add a second
parameter with the format. For instance the following example will force the
uncompressed XML format:

    var file = new osmium.File("some_file", "osm");

This will force PBF:

    var file = new osmium.File("some_file", "pbf");

You can read from STDIN, but you have to set the format then:

    var file = new osmium.File("-", "pbf");

You can also read directly from a URL like this:

    var file = new osmium.File("http://example.com/data.osm", "osm");

### Accessing OSM Data From a Node Buffer

Sometimes you have OSM data not in a file on disk but somewhere else. If you
can get it into a `node.Buffer` you can get it into Osmium. Simply give the
buffer to the `osmium.File` constructor instead of a file name:

    var buffer = new node.Buffer();
    // fill buffer with data
    var file = osmium.File(buffer, "pbf"); // buffer contents in PBF format

## Creating a Reader

Once you have defined a `File` you can open a `Reader` to access its data:

    var file = new osmium.File("data.osm");
    var reader = new osmium.Reader(file);

In this simple case you can also directly open the `Reader` with a file name:

    var reader = new osmium.Reader("data.osm");

But if you need to specify a format or want to read from a `node.Buffer` you
have to initialize the `osmium.File` first.

### Defining Which Object Types to Read

OSM files usually contain nodes, ways, and/or relations. Some special OSM files
can contain changelog entries. If you only want to read some of those object
types, you can specify which by adding an extra object parameter when
initializing the Reader.

In case you are only interested in nodes and ways it would look like this:

    var reader = new osmium.Reader("data.osm", { node: true, way: true });

The default is to read all the different object types. But to improve
performance it is recommened to only read the object types you really need.
That way other object types are discarded on the C++ side avoiding the costly
C++-to-Javascript conversion.

### Getting the File Header

The `Reader` object gives you access to the OSM file header:

    var reader = new osmium.Reader("data.osm");
    var header = reader.header();

The `header` object contains the `generator`, the software that created this
file:

    header.generator; // ===> 'CGImap 0.2.0'

And it contains the bounding box(es) of the data. (OSM files can have zero
or more bounding boxes, usually they have one.)

    var bounds = header.bound[0];
    var min_lon = bounds.left();
    var max_lon = bounds.right();
    var min_lat = bounds.bottom();
    var max_lat = bounds.top();

## Defining a Handler

Osmium will generate events for each object it reads. Those events can be
handled by your code. For this you need to define a handler and define some
callbacks on it:

    var handler = new osmium.Handler();
    handler.on('node', function(node) {
        console.log("got a node with id ", node.id);
    });

There are callbacks like the above for each type of OSM object: `node`, `way`,
`relation`, and `changeset`.

Sometimes you need to run initialization or finalization code. Use the `init`
and `done` callbacks:

    handler.on('init', function() { ... });
    handler.on('done', function() { ... });

There are also "before" and "after" callbacks called between objects of
different types:

    handler.on('before_nodes', function() { ... });
    handler.on('after_nodes', function() { ... });
    handler.on('before_ways', function() { ... });
    ...

### Handler Options

Sometimes you are only interested in nodes with tags. But the majority of
nodes do not have any tags at all. As an optimization you can tell the
handler to only give you tagged nodes:

    var handler = new osmium.Handler();
    handler.options({ 'tagged_nodes_only': true });

## Accessing OSM Objects

Through the `node`, `way` and `relation` callbacks you get access to the
OSM objects.

OSM objects are read-only and you can not create them yourself. Think of
them not as real objects, but convenient accessors to the data in the OSM
file. You can not keep the objects around outside the handler callback.
If you need to retain some data from the objects, extract the data you need
and put it in your own data structure. OSM files can be quite large, so
make sure you'll store the data efficiently.

### Accessing OSM Object Attributes

Each object has the usual attributes:

    var handler = new osmium.Handler();
    handler.on('node', function(node) {
        console.log(node.id);        // unique id of this object
        console.log(node.version);   // version of this object
        console.log(node.changeset); // changeset id of this object
        console.log(node.uid);       // user id
        console.log(node.user);      // name of user
    });

Accessing the timestamp when this object version was created is a bit more
complex. To get a `Date` object with this information call the `timestamp()`
function:

        console.log(node.timestamp());

Because this is an expensive operation and often a full `Date` object is not
needed, you can also get the timestamp as a simple number counting the seconds
since midnight January 1, 1970:

        console.log(node.timestamp_seconds_since_epoch);

The `node.visible` attribute tells you whether an object is visible or has
been deleted. For normal OSM files it is always true, but if the file contains
old versions of the OSM data ("history file") or is a "change file" (`.osc`),
this attribute can be true or false.

The same works for ways and relations. Changelog objects are different and not
documented here.

### Accessing the Tags

Of course you can also get access to the tags:

    way.tags(); // ==> { "highway": "residential", "maxspeed": "50" }

or ask for a specific key:

    way.tags("highway"); // ==> "residential"

Use the second form if you are only interested in a few tags, because it is
faster.

### Accessing Node Locations And the Coordinates Property

OSM node objects contain the location of the node, the coordinates. You can
access them in several ways:

    node.lon;           // ==> Number between -180.0 and 180.0
    node.lat;           // ==> Number between -90.0 and 90.0
    node.coordinates;   // ==> Coordinates object

The `Coordinates` object returned by the `coordinates` property has `lon`
and `lat` properties:

    var c = node.coordinates;
    console.log(c.lon);
    console.log(c.lat);

Unlike the `Node` object, the `Coordinates` object can created by you and it
can be copied around and used like any normal Javascript object.

### Accessing Way Nodes

Ways have a reference to the IDs of the nodes they use:

    way.node_refs();  // ==> [12345, 629375, 273054] (Array with node IDs)
    way.node_refs(1); // ==> 629375  (ID of 2nd node)

You can not access a node object this way, only the ID. See below for accessing
the node locations.

If all you need is the number of referenced nodes, use the `nodes_count`
property of the way object:

    way.nodes_count;  // ==> number of nodes in this way

### Accessing Relation Members

Relations have data about their members. To access them use the `members()`
function:

    relation.members();   // ==> Array of members
    relation.members(2);  // ==> Third member

A member is currently an Array with three values: The type of the member
('n', 'w', or 'r'), the ID of the member, and the role of the member. (This
interface will probably change at some point.)

If all you need is the number of members, use the `members_count` property of
the relation object:

    relation.members_count;  // ==> number of members in this way

## The LocationHandler

OSM ways contain only references to the nodes, but in most cases you don't need
the ID but the location. This is where the `LocationHandler` comes into play.
Initialize and call the `LocationHandler` like this:

    var reader = new osmium.Reader("some_file.osm");
    var location_handler = new osmium.LocationHandler();
    var handler = new osmium.Handler();
    // set up your handler callbacks
    osmium.apply(reader, location_handler, handler);

This will call the location handler before your handler. When the location
handler encounters nodes, it will store their location. Later, reading the
same file it will "add" the locations to the ways so that your handler will
see ways with added node locations. You can access them like this:

    way.node_coordinates();

This will return an `Array` with `osmium.Coordinates` objects. It is also
the basis for the geometry functions described below.

The `LocationHandler` can use different strategies for storing the node
locations. Which strategy is the best depends on the size of the input file and
the amount of main memory you have. Here are the available options:

 * "sparsetable" (default) - use this for small (city) to medium (country) sized
   data files.
 * "stlmap" - memory efficient for very small datasets, use for very small data
   files (small cities) or when "sparsetable" is not available.
 * "array" - best memory efficiency for large countries and planet sized data
   files. You will need main memory of size (8 bytes times the highest node ID),
   for a planet thats currently on the order of 25 GByte main memory! Not
   available on OSX!
 * "disk" - best memory efficiency for large countries and planet sized data
   files, but uses hard disk instead of memory. Use for very large data files
   if you are on OSX (and therefore can't use the "array" type) or if you don't
   have enough main memory. Will, of course, be slow compared to the other
   strategies.

To set the strategy initialize the `LocationHandler` with its name:

    var location_handler = new osmium.LocationHandler("array");

## Geometry Functions

Usually you don't want (arrays of) raw coordinates, but you want geometries in
some standard format. Osmium supports WKT (Well Known Text), WKB (Well Known
Binary) and GeoJSON representations of geometries for nodes and ways. You have
to use a LocationHandler as described above for these functions to work for
ways.

### WKT

The `wkt()` function called on a node or way returns a `String` with a
representation of the geometry:

    node.wkt(); // ===> "POINT(1.56 9.20)"
    way.wkt();  // ===> "LINESTRING(1.56 9.20, 4.56 10.29)"

See http://en.wikipedia.org/wiki/Well-known_text for a description of the
WKT and WKB format.

### WKB

The `wkb()` function called on a node or way returns a `node.Buffer` with a
binary representation of the geometry.

    node.wkb();
    way.wkb();

See http://en.wikipedia.org/wiki/Well-known_text for a description of the
WKT and WKB format.

### GeoJSON

The `geojson` function called on a node or way returns an Object with a
representation of the geometry according to the GeoJSON spec:

    node.geojson();

will result in something like this:

    {
        type: 'Point',
        coordinates: [-120.1891610, 48.4655800]
    }

and

    way.geojson();

will result in something like this:

    {
        type: 'LineString',
        coordinates: [
            [-120.1796227, 48.4798110],
            [-120.1787663, 48.4802976]
        ]
    }

This is, of course, only the "geometry part" of a full feature, you have to
add the rest yourself. See the `demo/geojson-stream` example for a complete
program using the GeoJSON function.

## A Complete Example

Finally here is a complete example to get you started: This parses an OSM file
and creates a node handler callback to count the total number of nodes:

    var osmium = require('osmium');
    var reader = new osmium.Reader("test/data/winthrop.osm");
    var handler = new osmium.Handler();
    var nodes = 0;
    handler.on('node', function(node) {
        ++nodes;
    });
    osmium.apply(reader, handler);
    console.log(nodes);

Result:

    1525

