# public-transport

Reads OSM file and outputs public transport route into a spatialite database.

Public transport is often tagged differently in different places, this one
works in Berlin, Germany.

## Install

    npm install

## Usage

    wget http://download.geofabrik.de/europe/germany/berlin-latest.osm.pbf
    ./index.js berlin-latest.osm.pbf berlin.db

You will get a spatialite database with table `routes`.

