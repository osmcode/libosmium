
SET(DART_TESTING_TIMEOUT 5)
INCLUDE(Dart)

include_directories("include")

IF(CMAKE_COMPILER_IS_GNUCC)
  set(CXX_FLAGS_WARNINGS "-Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long")
  set(CXXFLAGS "-g")
  SET(CMAKE_CXX_FLAGS "${CXXFLAGS} ${CXX_FLAGS_WARNINGS}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

MESSAGE(STATUS ${CMAKE_CXX_FLAGS})

add_library(testlib STATIC test_main.cpp)



macro(MORE_TESTS) 
FOREACH(test_name ${TESTS})
  MESSAGE(${test_name})
  add_executable(${test_name} ${FOLDER}/${test_name}.cpp)
  target_link_libraries (${test_name} testlib ${LIBS})
  ADD_TEST(${test_name} ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDFOREACH(test_name)
endmacro()


SET(FOLDER t/area)
SET(TESTS test_area_id test_node_ref_segment)
SET(LIBS "")
MORE_TESTS()

SET(FOLDER t/basic)
SET(TESTS test_entity_bits test_box test_changeset 
   test_location test_node test_way test_relation 
   test_node_ref test_timestamp
   test_object_comparisons)
SET(LIBS "")
MORE_TESTS()


SET(FOLDER t/buffer)
SET(TESTS test_buffer_node)
SET(LIBS "")
MORE_TESTS()

if (GEOS_FOUND AND PROJ_FOUND)
SET(FOLDER t/geom)
SET(TESTS
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
SET(LIBS ${GEOS_LIBRARY} ${PROJ_LIBRARY} ${BZIP2_LIBRARY})
MORE_TESTS()
else()
MESSAGE("Skipped geom tests - needed GEOS and PROJ libraries")
endif()

SET(FOLDER t/index)
SET(TESTS
   test_id_to_location
   test_typed_mmap
)
SET(LIBS "")
MORE_TESTS()

SET(FOLDER t/io)
SET(TESTS
   test_bzip2
   test_file_formats
)
SET(LIBS ${BZIP2_LIBRARIES})
MORE_TESTS()

SET(FOLDER t/tags)
SET(TESTS
   test_filter
   test_operators
   test_tag_list
)
SET(LIBS "")
MORE_TESTS()

SET(FOLDER t/util)
SET(TESTS
   test_options
)
SET(LIBS "")
MORE_TESTS()
