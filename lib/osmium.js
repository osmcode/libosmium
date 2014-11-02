var osmium = require('./binding/osmium.node');
exports = module.exports = osmium;
exports.version = require('../package').version;

exports.OSMObject.prototype.timestamp = function() {
    return new Date(1000 * this.timestamp_seconds_since_epoch);
}

exports.Changeset.prototype.created_at = function() {
    return new Date(1000 * this.created_at_seconds_since_epoch);
}

exports.Changeset.prototype.closed_at = function() {
    if (this.closed_at_seconds_since_epoch == 0) {
        return undefined;
    }
    return new Date(1000 * this.closed_at_seconds_since_epoch);
}

exports.Coordinates = function(lon, lat) {
    this.lon = lon;
    this.lat = lat;
    this.valid = function() {
        return this.lon !== undefined &&
               this.lat !== undefined &&
               this.lon >= -180.0 &&
               this.lon <= 180.0 &&
               this.lat >= -90.0 &&
               this.lat <= 90.0;
    };
}

exports.Box = function(bottom_left, top_right) {
    this.bottom_left = bottom_left;
    this.top_right = top_right;

    this.left   = function() { return this.bottom_left === undefined ? undefined : this.bottom_left.lon; };
    this.bottom = function() { return this.bottom_left === undefined ? undefined : this.bottom_left.lat; };
    this.right  = function() { return this.top_right   === undefined ? undefined : this.top_right.lon; };
    this.top    = function() { return this.top_right   === undefined ? undefined : this.top_right.lat; };
}

exports.Node.prototype.geojson = function() {
    return {
        type: 'Point',
        coordinates: [this.lon, this.lat]
    };
}

exports.Way.prototype.geojson = function() {
    return {
        type: 'LineString',
        coordinates: this.node_coordinates().map(function(c) {
            return [ c.lon, c.lat ];
        })
    };
}

exports.Converter = function(args) {
    if (args === undefined) {
        args = {};
    }

    this.show_layers = args.show_layers ? true : false;
    this.output_name = args.output || "out.db";

    function Layer(name) {
        this.name = name;
        this.geom_type = 'POINT';
        this.attributes = [];
        this.attribute_names = {};

        this.of_type = function(type) {
            this.geom_type = type.toUpperCase();
            if (this.geom_type != 'POINT' && this.geom_type != 'LINESTRING' /*&& this.geom_type != 'POLYGON'*/) {
                throw new Error('Unknown geometry type: ' + type);
            }
            return this;
        };

        this.with_attribute = function(name, type) {
            var attribute_type = type.toUpperCase();
            if (attribute_type != 'INTEGER' && attribute_type != 'STRING' && attribute_type != 'BOOL' && attribute_type != 'REAL') {
                throw new Error('Unknown attribute type: ' + type);
            }
            var attribute = { name: name, type: attribute_type };
            this.attributes.push(attribute);
            this.attribute_names[name] = attribute;
            return this;
        }
    }

    var layers = {};

    function Rule(type, key, value) {
        this.type = type;
        this.key = key;

        if (value == '*') {
            this.value = null;
        } else {
            this.value = value;
        }

        this.layer = null;
        this.attrs = {};

        this.to_layer = function(name) {
            if (! layers[name]) {
                throw new Error('Unknown layer: ' + name);
            }
            this.layer = name;
            return this;
        };

        this.attr = function(attr, key) {
            if (this.layer == null) {
                throw new Error('Layer not set for rule ' + key + '=' + value);
            }

            if (! layers[this.layer].attribute_names[attr]) {
                throw new Error("There is no attribute named '" + attr + "' in layer '" + this.layer + "'");
            }

            if (key instanceof Function) {
                this.attrs[attr] = key;
            } else if (key == null) {
                this.attrs[attr] = function(tags) {
                    return tags[attr];
                };
            } else {
                this.attrs[attr] = function(tags) {
                    return tags[key];
                };
            }

            return this;
        };


        if (typeof(value) == 'string' && value.match(/\|/)) {
            value = value.split('|');
        }

        if (value == null) {
            this.match = function(obj) {
                return !!obj.tags(key);
            };
        } else if (typeof(value) == 'string') {
            this.match = function(obj) {
                return obj.tags(key) == value;
            };
        } else if (value instanceof Array) {
            this.match = function(obj) {
                var val = obj.tags(key);
                if (! val) {
                    return false;
                }
                return value.indexOf(val) != -1;
            };
        } else if (value instanceof RegExp) {
            this.match = function(obj) {
                return obj.tags(key) && value.test(obj.tags(key));
            }
        } else {
            throw new Error('Can not understand rule');
        }
    }

    this.add_layer = function(name) {
        var layer = new Layer(name);
        layers[name] = layer;
        return layer;
    }

    var rules = {
        node: [],
        way:  [],
        area: []
    };

    this.rule = function(type, key, value) {
        var rule = new Rule(type, key, value);
        rules[type].push(rule);
        return rule;
    }

    this.node = function(key, value) {
        return this.rule('node', key, value);
    }

    this.way = function(key, value) {
        return this.rule('way', key, value);
    }

    this.area = function(key, value) {
        return this.rule('area', key, value);
    }

    function tags2attributes(id, tags, attrs) {
        var obj = { osm_id: id };
        for (var a in attrs) {
            obj[a] = attrs[a](tags);
        }
        return obj;
    }

    function convert(osm_object) {
        rules[osm_object.type].forEach(function(rule) {
            if (rule.match(osm_object)) {
                var attributes = tags2attributes(osm_object.id, osm_object.tags(), rule.attrs);
                var p = [osm_object.wkb()];
                var layer = layers[rule.layer];
                layer.attributes.forEach(function(attribute) {
                    p.push(attributes[attribute.name]);
                });
                layer.insert.run.apply(layer.insert, p);
            }
        });
    }

    var handler = new osmium.Handler;
    handler.on('node', convert);
    handler.on('way', convert);
    // handler.on('area', convert);

    this.convert = function(input_name) {

        var fs = require('fs');
        if (fs.existsSync(this.output_name)) {
            console.log("Output database file '" + this.output_name + "' exists. Refusing to overwrite it.");
            process.exit(1);
        }

        // setup database
        var sqlite = require('spatialite');
        var db = new sqlite.Database(this.output_name);
        db.serialize();

        db.spatialite();
        db.run("PRAGMA synchronous = OFF;"); // otherwise it is very slow
        db.run("SELECT InitSpatialMetaData('WGS84');");

        var show_layers = this.show_layers;

        // setup layers in database
        for (var t in layers) {
            var layer = layers[t];

            if (show_layers) {
                console.log('Layer: ' + layer.name + ' (' + layer.geom_type + ')');
            }

            db.run("CREATE TABLE " + layer.name + " (id INTEGER PRIMARY KEY);");
            db.run("SELECT AddGeometryColumn('" + layer.name + "', 'geom', 4326, '" + layer.geom_type + "', 2);");

            var insert = "INSERT INTO " + layer.name + " (geom";
            var qm = '';

            layer.attributes.forEach(function(attribute) {
                if (show_layers) {
                    console.log('  ' + (attribute.name + '          ').substr(0, 11) + attribute.type);
                }
                db.run("ALTER TABLE " + layer.name + " ADD COLUMN " + attribute.name + " " + attribute.type + ";");
                insert += ", " + attribute.name;
                qm += ", ?";
            });

            insert += ") VALUES (GeomFromWKB(?, 4326)" + qm + ");";

            layer.insert = db.prepare(insert);
            if (show_layers) {
                console.log('');
            }
        }

        // convert data
        var reader = new osmium.Reader(input_name);
        var location_handler = new osmium.LocationHandler();
        osmium.apply(reader, location_handler, handler);
        reader.close();

        // cleanup
        for (var layer in layers) {
            layers[layer].insert.finalize();
        }
        db.close();
    };

};

