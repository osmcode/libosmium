message(STATUS "Configuring unit tests...")

set(DART_TESTING_TIMEOUT 60)
include(Dart)
include_directories(include)

add_library(testlib STATIC test_main.cpp)

#----------------------------------------------------------------------
macro(compile_tests)
    foreach(test_name ${TESTS})
        add_executable(${test_name} t/${TEST_GROUP}/${test_name}.cpp)
        target_link_libraries(${test_name} testlib ${OSMIUM_LIBRARIES})
        add_test(${test_name} ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach(test_name)
endmacro()


#----------------------------------------------------------------------
set(TEST_GROUP area)
set(TESTS
    test_area_id
    test_node_ref_segment
)
compile_tests()

set(TEST_GROUP basic)
set(TESTS
    test_box
    test_changeset
    test_entity_bits
    test_location
    test_node
    test_node_ref
    test_object_comparisons
    test_relation
    test_timestamp
    test_way
)
compile_tests()


set(TEST_GROUP buffer)
set(TESTS
    test_buffer_node
    test_buffer_purge
)
compile_tests()

if(GEOS_FOUND AND PROJ_FOUND)
    set(TEST_GROUP geom)
    set(TESTS
        test_factory_with_projection
        test_geojson
        test_geos
        test_geos_wkb
        test_mercator
        test_projection
        test_wkb
        test_wkt
    )
    if(GDAL_FOUND)
      list(APPEND TESTS test_ogr)
    endif()
    compile_tests()
else()
    message("Skipped geom tests - needed GEOS and PROJ libraries")
endif()

set(TEST_GROUP index)
set(TESTS
    test_typed_mmap
)
if(SPARSEHASH_FOUND)
    list(APPEND TESTS test_id_to_location)
else()
    message("Skipped index/test_id_to_location test - needed Google SparseHash")
endif()
compile_tests()

set(TEST_GROUP io)
set(TESTS
    test_bzip2
    test_file_formats
)
compile_tests()
set(TESTS
    test_reader
)
compile_tests()

set(TEST_GROUP tags)
set(TESTS
    test_filter
    test_operators
    test_tag_list
)
compile_tests()

set(TEST_GROUP util)
set(TESTS
    test_options
)
compile_tests()

