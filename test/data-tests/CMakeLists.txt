message(STATUS "Configuring data tests...")

if(NOT GDAL_FOUND OR NOT EXPAT_FOUND)
    message("Sorry, building data tests needs GDAL and Expat")
    return()
endif()

set(DART_TESTING_TIMEOUT 300)
include(Dart)

include_directories("include")
include_directories("../include")

message(STATUS "Looking for osm-testdata")
find_path(OSM_TESTDATA grid/data/all.osm HINT ../../../osm-testdata)
if (OSM_TESTDATA STREQUAL "OSM_TESTDATA-NOTFOUND")
    message(STATUS "Looking for osm-testdata - not found (data tests disabled)")
    return()
endif()
message(STATUS "Looking for osm-testdata - found")

#----------------------------------------------------------------------

file(GLOB TESTCASE_CPPS testcases/*.cpp)
add_executable(testdata-testcases testdata-testcases.cpp ${TESTCASE_CPPS})
target_link_libraries(testdata-testcases
                      ${OSMIUM_XML_LIBRARIES}
)
add_test(NAME testdata-testcases
         COMMAND testdata-testcases
)
set_tests_properties(testdata-testcases
    PROPERTIES ENVIRONMENT "TESTCASES_DIR=${OSM_TESTDATA}/grid/data")

#----------------------------------------------------------------------

add_executable(testdata-overview testdata-overview.cpp)
target_link_libraries(testdata-overview
                      ${OSMIUM_XML_LIBRARIES}
                      ${GDAL_LIBRARIES}
)
add_test(NAME testdata-overview
         COMMAND testdata-overview ${OSM_TESTDATA}/grid/data/all.osm
)

#----------------------------------------------------------------------

add_executable(testdata-xml testdata-xml.cpp)
target_link_libraries(testdata-xml
                      ${OSMIUM_XML_LIBRARIES}
)
add_test(NAME testdata-xml
         COMMAND testdata-xml
)
set_tests_properties(testdata-xml
    PROPERTIES ENVIRONMENT "TESTDIR=${OSM_TESTDATA}/xml/data")

#----------------------------------------------------------------------

add_executable(testdata-multipolygon testdata-multipolygon.cpp)
target_link_libraries(testdata-multipolygon
                      ${OSMIUM_XML_LIBRARIES}
                      ${GDAL_LIBRARIES}
)

if(WIN32)
    set(MULTIPOLYGON_TEST_SCRIPT "run-testdata-multipolygon.bat")
else()
    set(MULTIPOLYGON_TEST_SCRIPT "run-testdata-multipolygon.sh")
endif()

if(MSVC)
    set(EXE_DIR ${CMAKE_BUILD_TYPE})
else()
    set(EXE_DIR .)
endif()

add_test(NAME testdata-multipolygon
         COMMAND ${CMAKE_SOURCE_DIR}/test/data-tests/${MULTIPOLYGON_TEST_SCRIPT}
         ${OSM_TESTDATA}
         ${EXE_DIR}
)

#----------------------------------------------------------------------
