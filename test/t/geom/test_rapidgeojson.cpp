#include "catch.hpp"

#include <osmium/geom/rapid_geojson.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "area_helper.hpp"
#include "wnl_helper.hpp"

typedef rapidjson::Writer<rapidjson::StringBuffer> writer_type;
rapidjson::StringBuffer stream;
writer_type writer{stream};
osmium::geom::RapidGeoJSONFactory<writer_type> factory{writer};

TEST_CASE("RapidGeoJSON point geometry") {

    SECTION("point") {
        osmium::Location location{3.2, 4.2};
        rapidjson::Document document = factory.create_point(location);

        REQUIRE(std::string{"Point"} == document["type"].GetString());

        const rapidjson::Value& coordinates = document["coordinates"];
        REQUIRE(coordinates[0].GetDouble() == location.lon());
        REQUIRE(coordinates[1].GetDouble() == location.lat());
    }

    SECTION("writing point as string") {
        rapidjson::StringBuffer stream;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stream);

        osmium::Location location{3.2, 4.2};
        rapidjson::Document document = factory.create_point(location);
        document.Accept(writer);

        const std::string json = stream.GetString();
        REQUIRE(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"} == json);
    }

    SECTION("empty_point") {
        REQUIRE_THROWS_AS(factory.create_point(osmium::Location{}), const osmium::invalid_location&);
    }

}

TEST_CASE("RapidGeoJSON linestring geometry") {
    SECTION("linestring, default") {
        // const auto& wnl = create_test_wnl_okay(buffer);
        // const rapidjson::Document = factory.create_linestring(wnl);

        // REQUIRE(std::string{"LineString"} == document["type"].GetString());

        // REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

}
