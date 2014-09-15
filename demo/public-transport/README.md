# public-transport

Reads OSM file and outputs public transport route into a GeoJSON file.

Public transport is often tagged differently in different places, this one
works well in Berlin, Germany.

## Install

    npm install

## Usage

    wget http://download.geofabrik.de/europe/germany/berlin-latest.osm.pbf
    ./index.js berlin-latest.osm.pbf berlin.geojson

