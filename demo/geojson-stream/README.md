

# Install

    npm install


# Usage

Convert osm data to geojson file:

    ./index.js --output winthrop.geojson --input ../../test/data/winthrop.osm

You can also pipe geojson to stdout and push to http://geojson.io:

    npm install -g geojsonio-cli
    ./index.js --output stdout --input ../../test/data/winthrop.osm | geojsonio
