/*

  An example for the use of mercator projection and tiles.

  Call with ZOOM, LON, and LAT arguments.

  The code in this example file is released into the Public Domain.

*/

#include <cstdlib>
#include <iostream>

#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/tile.hpp>
#include <osmium/osm/location.hpp>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " ZOOM LON LAT\n";
        exit(1);
    }

    const int zoom = atoi(argv[1]);

    if (zoom < 0 || zoom > 30) {
        std::cerr << "ERROR: Zoom must be between 0 and 30\n";
        exit(1);
    }

    const double lon = atof(argv[2]);
    const double lat = atof(argv[3]);

    const osmium::Location location{lon, lat};

    std::cout << "WGS84:    lon=" << lon << " lat=" << lat << "\n";

    if (!location.valid()) {
        std::cerr << "ERROR: Location is invalid\n";
        exit(1);
    }

    const osmium::geom::Coordinates c = osmium::geom::lonlat_to_mercator(location);
    std::cout << "Mercator: x=" << c.x << " y=" << c.y << "\n";

    const osmium::geom::Tile tile(zoom, location);
    std::cout << "Tile:     zoom=" << tile.z << " x=" << tile.x << " y=" << tile.y << "\n";
}

