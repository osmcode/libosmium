#include "catch.hpp"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <osmium/geom/rapid_geojson.hpp>
#include "area_helper.hpp"
#include "wnl_helper.hpp"

typedef rapidjson::Writer<rapidjson::StringBuffer> writer_type;

TEST_CASE("RapidGeoJSON point geometry") {

    SECTION("basic point") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        osmium::Location location{3.2, 4.2};

        factory.create_point(location);

        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"} == json);

        stream.Clear();
        writer.Reset(stream);
        factory.create_point(location);

        const std::string json2 = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"} == json2);
    }

    SECTION("empty_point") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};
        REQUIRE_THROWS_AS(factory.create_point(osmium::Location{}), const osmium::invalid_location&);
    }

}

TEST_CASE("RapidGeoJSON linestring geometry") {
    osmium::memory::Buffer buffer{1000};

    SECTION("linestring, default") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_okay(buffer);
        factory.create_linestring(wnl);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    SECTION("linestring, unique, backwards") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_okay(buffer);
        factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.2,4.2]]}"} == json);
    }

    SECTION("linestring, all") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_okay(buffer);
        factory.create_linestring(wnl, osmium::geom::use_nodes::all);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    SECTION("linestring, all, backwards") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_okay(buffer);
        factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.5,4.7],[3.2,4.2]]}"} == json);
    }

    SECTION("empty_linestring") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_empty(buffer);

        REQUIRE_THROWS_AS(factory.create_linestring(wnl), const osmium::geometry_error&);
        REQUIRE_THROWS_AS(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), const osmium::geometry_error&);
        REQUIRE_THROWS_AS(factory.create_linestring(wnl, osmium::geom::use_nodes::all), const osmium::geometry_error&);
        REQUIRE_THROWS_AS(factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward), const osmium::geometry_error&);
    }

    SECTION("linestring with two same locations") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};
        const auto& wnl = create_test_wnl_same_location(buffer);

        REQUIRE_THROWS_AS(factory.create_linestring(wnl), const osmium::geometry_error&);
        REQUIRE_THROWS_AS(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), const osmium::geometry_error&);

        {
            rapidjson::StringBuffer stream;
            writer_type writer{stream};
            osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

            factory.create_linestring(wnl, osmium::geom::use_nodes::all);
            const std::string json = stream.GetString();
            REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"} == json);
        }

        {
            rapidjson::StringBuffer stream;
            writer_type writer{stream};
            osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

            factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward);
            const std::string json = stream.GetString();
            REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"} == json);
        }
    }

    SECTION("linestring with undefined location") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const auto& wnl = create_test_wnl_undefined_location(buffer);
        REQUIRE_THROWS_AS(factory.create_linestring(wnl), const osmium::invalid_location&);
    }

}

TEST_CASE("RapidGeoJSON polygon geometry") {
    osmium::memory::Buffer buffer{1000};
    const auto& wnl = create_test_wnl_closed(buffer);

    SECTION("unique forwards (default)") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        factory.create_polygon(wnl);
        const std::string json = stream.GetString();
        REQUIRE(json == "{\"type\":\"Polygon\",\"coordinates\":[[[3.0,3.0],[4.1,4.1],[3.6,4.1],[3.1,3.5],[3.0,3.0]]]}");
    }

    SECTION("unique backwards") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        factory.create_polygon(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward);
        const std::string json = stream.GetString();
        REQUIRE(json == "{\"type\":\"Polygon\",\"coordinates\":[[[3.0,3.0],[3.1,3.5],[3.6,4.1],[4.1,4.1],[3.0,3.0]]]}");
    }

    SECTION("all forwards") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        factory.create_polygon(wnl,  osmium::geom::use_nodes::all);
        const std::string json = stream.GetString();
        REQUIRE(json == "{\"type\":\"Polygon\",\"coordinates\":[[[3.0,3.0],[4.1,4.1],[4.1,4.1],[3.6,4.1],[3.1,3.5],[3.0,3.0]]]}");
    }

    SECTION("all backwards") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        factory.create_polygon(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward);
        const std::string json = stream.GetString();
        REQUIRE(json == "{\"type\":\"Polygon\",\"coordinates\":[[[3.0,3.0],[3.1,3.5],[3.6,4.1],[4.1,4.1],[4.1,4.1],[3.0,3.0]]]}");
    }
}

TEST_CASE("RapidGeoJSON area geometry") {
    osmium::memory::Buffer buffer{1000};

    SECTION("area_1outer_0inner") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const osmium::Area& area = create_test_area_1outer_0inner(buffer);

        REQUIRE_FALSE(area.is_multipolygon());
        REQUIRE(std::distance(area.cbegin(), area.cend()) == 2);
        REQUIRE(area.subitems<osmium::OuterRing>().size() == area.num_rings().first);

        factory.create_multipolygon(area);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[3.2,4.2],[3.5,4.7],[3.6,4.9],[3.2,4.2]]]]}"} == json);
    }

    SECTION("area_1outer_1inner") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const osmium::Area& area = create_test_area_1outer_1inner(buffer);

        REQUIRE_FALSE(area.is_multipolygon());
        REQUIRE(std::distance(area.cbegin(), area.cend()) == 3);
        REQUIRE(area.subitems<osmium::OuterRing>().size() == area.num_rings().first);
        REQUIRE(area.subitems<osmium::InnerRing>().size() == area.num_rings().second);

        factory.create_multipolygon(area);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1.0,1.0],[8.0,1.0],[8.0,8.0],[1.0,8.0],[1.0,1.0]]]]}"} == json);
    }

    SECTION("area_2outer_2inner") {
        rapidjson::StringBuffer stream;
        writer_type writer{stream};
        osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

        const osmium::Area& area = create_test_area_2outer_2inner(buffer);

        REQUIRE(area.is_multipolygon());
        REQUIRE(std::distance(area.cbegin(), area.cend()) == 5);
        REQUIRE(area.subitems<osmium::OuterRing>().size() == area.num_rings().first);
        REQUIRE(area.subitems<osmium::InnerRing>().size() == area.num_rings().second);

        int outer_ring=0;
        int inner_ring=0;
        for (const auto& outer : area.outer_rings()) {
            if (outer_ring == 0) {
                REQUIRE(outer.front().ref() == 1);
            } else if (outer_ring == 1) {
                REQUIRE(outer.front().ref() == 100);
            } else {
                REQUIRE(false);
            }
            for (const auto& inner : area.inner_rings(outer)) {
                if (outer_ring == 0 && inner_ring == 0) {
                    REQUIRE(inner.front().ref() == 5);
                } else if (outer_ring == 0 && inner_ring == 1) {
                    REQUIRE(inner.front().ref() == 10);
                } else {
                    REQUIRE(false);
                }
                ++inner_ring;
            }
            inner_ring = 0;
            ++outer_ring;
        }

        factory.create_multipolygon(area);
        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1.0,1.0],[4.0,1.0],[4.0,4.0],[1.0,4.0],[1.0,1.0]],[[5.0,5.0],[5.0,7.0],[7.0,7.0],[5.0,5.0]]],[[[10.0,10.0],[11.0,10.0],[11.0,11.0],[10.0,11.0],[10.0,10.0]]]]}"} == json);
    }

}
