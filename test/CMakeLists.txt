
set(DART_TESTING_TIMEOUT 5)
include(Dart)

include_directories("include")

if(CMAKE_COMPILER_IS_GNUCC)
    set(CXX_FLAGS_WARNINGS "-Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long")
    set(CXXFLAGS "-g")
    set(CMAKE_CXX_FLAGS "${CXXFLAGS} ${CXX_FLAGS_WARNINGS}")
endif(CMAKE_COMPILER_IS_GNUCC)

message(STATUS ${CMAKE_CXX_FLAGS})

add_library(testlib STATIC test_main.cpp)


macro(MORE_TESTS) 
    foreach(test_name ${TESTS})
        message(${test_name})
        add_executable(${test_name} ${FOLDER}/${test_name}.cpp)
        target_link_libraries(${test_name} testlib ${LIBS})
        add_test(${test_name} ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach(test_name)
endmacro()


set(FOLDER t/area)
set(TESTS test_area_id test_node_ref_segment)
set(LIBS "")
MORE_TESTS()

set(FOLDER t/basic)
set(TESTS test_entity_bits test_box test_changeset 
   test_location test_node test_way test_relation 
   test_node_ref test_timestamp
   test_object_comparisons)
set(LIBS "")
MORE_TESTS()


set(FOLDER t/buffer)
set(TESTS test_buffer_node)
set(LIBS "")
MORE_TESTS()

if(GEOS_FOUND AND PROJ_FOUND)
    set(FOLDER t/geom)
    set(TESTS
        test_factory_with_projection
        test_geojson
        test_geos
        test_geos_wkb
        test_mercator
#   test_ogr
        test_projection
        test_wkb
        test_wkt
    )
    set(LIBS ${GEOS_LIBRARY} ${PROJ_LIBRARY} ${BZIP2_LIBRARY})
    MORE_TESTS()
else()
    message("Skipped geom tests - needed GEOS and PROJ libraries")
endif()

set(FOLDER t/index)
set(TESTS
   test_id_to_location
   test_typed_mmap
)
set(LIBS "")
MORE_TESTS()

set(FOLDER t/io)
set(TESTS
   test_bzip2
   test_file_formats
)
set(LIBS ${BZIP2_LIBRARIES})
MORE_TESTS()

set(FOLDER t/tags)
set(TESTS
   test_filter
   test_operators
   test_tag_list
)
set(LIBS "")
MORE_TESTS()

set(FOLDER t/util)
set(TESTS
   test_options
)
set(LIBS "")
MORE_TESTS()

