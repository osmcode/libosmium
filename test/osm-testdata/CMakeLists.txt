message(STATUS "Configuring osm-testdata tests...")

if(NOT GDAL_FOUND OR
   NOT GEOS_FOUND OR
   NOT OSMPBF_FOUND OR
   NOT PROTOBUF_FOUND OR
   NOT EXPAT_FOUND)
    message("Sorry, building osm-testdata tests needs GEOS, GDAL, Expat, Protobuf and OSMPBF")
    return()
endif()

set(DART_TESTING_TIMEOUT 300)
include(Dart)

include_directories("include")
include_directories("../include")


get_filename_component(TESTDATA_DIR ../../../osm-testdata ABSOLUTE)
if(NOT EXISTS ${TESTDATA_DIR})
    execute_process(COMMAND git clone https://github.com/osmcode/osm-testdata.git ${TESTDATA_DIR})
endif()


#if(CMAKE_COMPILER_IS_GNUCC)
#    add_compile_options(
##    add_definitions(
#        $<$<CXX_COMPILER_ID:Clang>:-Wdocumentation>
#        $<$<CXX_COMPILER_ID:Clang>:-Wunused-exception-parameter>
#        $<$<CXX_COMPILER_ID:Clang>:-Wmissing-declarations>
#        $<$<CXX_COMPILER_ID:Clang>:-Weverything>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat-pedantic>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-unused-macros>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-exit-time-destructors>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-global-constructors>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-padded>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-switch-enum>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-prototypes>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-weak-vtables>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-cast-align>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-float-equal>
#    )
#endif(CMAKE_COMPILER_IS_GNUCC)

add_executable(testdata-overview testdata-overview.cpp)
target_link_libraries(testdata-overview ${OSMIUM_LIBRARIES})
add_test(testdata-overview testdata-overview ${TESTDATA_DIR}/grid/data/all.osm)

add_executable(testdata-multipolygon testdata-multipolygon.cpp)
target_link_libraries(testdata-multipolygon ${OSMIUM_LIBRARIES})
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
add_test(testdata-multipolygon
         ${CMAKE_SOURCE_DIR}/test/osm-testdata/${MULTIPOLYGON_TEST_SCRIPT}
         ${TESTDATA_DIR}
         ${EXE_DIR}
)

file(GLOB TESTCASE_CPPS testcases/*.cpp)
add_executable(testdata-testcases testdata-testcases.cpp ${TESTCASE_CPPS})
target_link_libraries(testdata-testcases ${OSMIUM_LIBRARIES})
add_test(testdata-testcases testdata-testcases)
set_tests_properties(testdata-testcases
    PROPERTIES ENVIRONMENT "TESTCASES_DIR=${TESTDATA_DIR}/grid/data")

add_executable(testdata-xml testdata-xml.cpp)
target_link_libraries(testdata-xml ${OSMIUM_LIBRARIES})
add_test(testdata-xml testdata-xml)
set_tests_properties(testdata-xml
    PROPERTIES ENVIRONMENT "TESTDIR=${TESTDATA_DIR}/xml/data")
