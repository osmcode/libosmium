if (NOT GDAL_FOUND OR NOT GEOS_FOUND OR NOT OSMPBF_FOUND OR NOT PROTOBUF_FOUND OR NOT EXPAT_FOUND)
    message("Sorry, building osm-testdata tests needs GEOS, GDAL, Expat, Protobuf and OSMPBF")
    return()
endif()

set(DART_TESTING_TIMEOUT 60)
include(Dart)

include_directories("include")
include_directories("../include")


get_filename_component(TESTDATA_DIR ../../../osm-testdata ABSOLUTE)
if(NOT EXISTS ${TESTDATA_DIR})
    execute_process(COMMAND git clone https://github.com/osmcode/osm-testdata.git ${TESTDATA_DIR})
endif()


if(CMAKE_COMPILER_IS_GNUCC)
    set(CXX_FLAGS_WARNINGS "-Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long")
    set(CXXFLAGS "-g")
    set(CMAKE_CXX_FLAGS "${CXXFLAGS} ${CXX_FLAGS_WARNINGS}")
endif(CMAKE_COMPILER_IS_GNUCC)

message(STATUS ${CMAKE_CXX_FLAGS})

set(LIBS ${EXPAT_LIBRARY} ${OSMPBF_LIBRARIES} ${PROTOBUF_LITE_LIBRARY}
    ${ZLIB_LIBRARY} ${BZIP2_LIBRARIES} ${GEOS_LIBRARY} ${GDAL_LIBRARY}
    ${WIN_LIBS} ${CMAKE_THREAD_LIBS_INIT})

file(REMOVE *.db)

add_executable(testdata-overview testdata-overview.cpp)
target_link_libraries(testdata-overview ${LIBS})
add_test(testdata-overview testdata-overview ${TESTDATA_DIR}/grid/data/all.osm)

add_executable(testdata-multipolygon testdata-multipolygon.cpp)
target_link_libraries(testdata-multipolygon ${LIBS})
add_test(testdata-multipolygon testdata-multipolygon "${TESTDATA_DIR}/grid/data/all.osm >multipolygon.log 2>&1")

file(GLOB TESTCASE_CPPS testcases/*.cpp)
add_executable(testdata-testcases testdata-testcases.cpp ${TESTCASE_CPPS})
target_link_libraries(testdata-testcases ${LIBS})
add_test(testdata-testcases testdata-testcases)
set_tests_properties(testdata-testcases
    PROPERTIES ENVIRONMENT "TESTCASES_DIR=${TESTDATA_DIR}/grid/data")

add_executable(testdata-xml testdata-xml.cpp)
target_link_libraries(testdata-xml ${LIBS})
add_test(testdata-xml testdata-xml)
set_tests_properties(testdata-xml
    PROPERTIES ENVIRONMENT "TESTDIR=${TESTDATA_DIR}/xml/data")
