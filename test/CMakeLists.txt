message(STATUS "Configuring unit tests...")

set(DART_TESTING_TIMEOUT 60)
include(Dart)

include_directories("include")

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
        -Wredundant-decls
        -Wdisabled-optimization
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wsign-promo
        -Wno-long-long
    )
    set(CXXFLAGS "-g")
    set(CMAKE_CXX_FLAGS "${CXXFLAGS}")
endif(CMAKE_COMPILER_IS_GNUCC)

add_library(testlib STATIC test_main.cpp)

#----------------------------------------------------------------------
macro(compile_tests)
    foreach(test_name ${TESTS})
        add_executable(${test_name} t/${TEST_GROUP}/${test_name}.cpp)
        target_link_libraries(${test_name} testlib ${LIBS})
        add_test(${test_name} ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach(test_name)
endmacro()


#----------------------------------------------------------------------
set(TEST_GROUP area)
set(TESTS
    test_area_id
    test_node_ref_segment
)
set(LIBS "")
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
set(LIBS "")
compile_tests()


set(TEST_GROUP buffer)
set(TESTS
    test_buffer_node
    test_buffer_purge
)
set(LIBS "")
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
    set(LIBS ${GEOS_LIBRARY} ${PROJ_LIBRARY} ${BZIP2_LIBRARY})
    if(GDAL_FOUND)
      list(APPEND TESTS test_ogr)
      list(APPEND LIBS ${GDAL_LIBRARY})
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
set(LIBS "")
compile_tests()

set(TEST_GROUP io)
set(TESTS
    test_bzip2
    test_file_formats
)
set(LIBS ${BZIP2_LIBRARIES})
compile_tests()
set(TESTS
    test_reader
)
set(LIBS
    ${EXPAT_LIBRARY}
    ${OSMPBF_LIBRARIES}
    ${PROTOBUF_LITE_LIBRARY}
    ${ZLIB_LIBRARY}
    ${BZIP2_LIBRARIES}
    ${WIN_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)
compile_tests()

set(TEST_GROUP tags)
set(TESTS
    test_filter
    test_operators
    test_tag_list
)
set(LIBS "")
compile_tests()

set(TEST_GROUP util)
set(TESTS
    test_options
)
set(LIBS "")
compile_tests()

