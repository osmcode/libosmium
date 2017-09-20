
#include "catch.hpp"

#include <osmium/geom/rapid_geojson_document.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <vector>

#include "area_helper.hpp"
#include "wnl_helper.hpp"

osmium::geom::RapidGeoJSONDocumentFactory<> factory;

std::string to_string(rapidjson::Document& document) {
    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stream);
    document.Accept(writer);
    return stream.GetString();
}

void verify_coords(std::vector<std::vector<double>> expected, const rapidjson::Value& coordinates) {
    for (int i = 0; i < expected.size(); i++) {
        std::vector<double> point = expected[i];
        REQUIRE(coordinates[i].GetArray()[0].GetDouble() == point[0]);
        REQUIRE(coordinates[i].GetArray()[1].GetDouble() == point[1]);
    }
}

TEST_CASE("RapidGeoJSON Document point geometry") {

    SECTION("point") {
        osmium::Location location{3.2, 4.2};
        rapidjson::Document document = factory.create_point(location);

        REQUIRE(std::string{"Point"} == document["type"].GetString());

        const rapidjson::Value& coordinates = document["coordinates"];
        REQUIRE(coordinates[0].GetDouble() == location.lon());
        REQUIRE(coordinates[1].GetDouble() == location.lat());
    }

    SECTION("writing point as string") {
        osmium::Location location{3.2, 4.2};
        rapidjson::Document document = factory.create_point(location);
        const std::string json = to_string(document);
        REQUIRE(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"} == json);
    }

    SECTION("empty_point") {
        REQUIRE_THROWS_AS(factory.create_point(osmium::Location{}), const osmium::invalid_location&);
    }

}

TEST_CASE("RapidGeoJSON Document linestring geometry") {
    osmium::memory::Buffer buffer{1000};

    SECTION("linestring, default") {
        const auto& wnl = create_test_wnl_okay(buffer);
        const rapidjson::Document document = factory.create_linestring(wnl);

        REQUIRE(std::string{"LineString"} == document["type"].GetString());

        const rapidjson::Value& coordinates = document["coordinates"];

        std::vector<std::vector<double>> expected {
            {3.2, 4.2},
            {3.5, 4.7},
            {3.6, 4.9}
        };
        verify_coords(expected, coordinates);
    }

    SECTION("Writing LineString as string") {
        const auto& wnl = create_test_wnl_okay(buffer);
        rapidjson::Document document = factory.create_linestring(wnl);
        const std::string json = to_string(document);
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    SECTION("linestring, unique, backwards") {
        const auto& wnl = create_test_wnl_okay(buffer);
        rapidjson::Document document = factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward);

        REQUIRE(std::string{"LineString"} == document["type"].GetString());
        const rapidjson::Value& coordinates = document["coordinates"];

        std::vector<std::vector<double>> expected {
            {3.6, 4.9},
            {3.5, 4.7},
            {3.2, 4.2}
        };
        verify_coords(expected, coordinates);
    }

    SECTION("linestring, all") {
        const auto& wnl = create_test_wnl_okay(buffer);
        const rapidjson::Document document = factory.create_linestring(wnl, osmium::geom::use_nodes::all);

        REQUIRE(std::string{"LineString"} == document["type"].GetString());
        const rapidjson::Value& coordinates = document["coordinates"];

        // REQUIRE(coordinates[0].GetArray()[0].GetDouble() == 3.2);
        // REQUIRE(coordinates[0].GetArray()[1].GetDouble() == 4.2);
        // REQUIRE(coordinates[1].GetArray()[0].GetDouble() == 3.5);
        // REQUIRE(coordinates[1].GetArray()[1].GetDouble() == 4.7);
        // REQUIRE(coordinates[2].GetArray()[0].GetDouble() == 3.2);
        // REQUIRE(coordinates[2].GetArray()[1].GetDouble() == 4.2);

        // REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    SECTION("linestring, all, backwards") {
        // const auto& wnl = create_test_wnl_okay(buffer);
        // const std::string json{factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        // REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.5,4.7],[3.2,4.2]]}"} == json);
    }


}
