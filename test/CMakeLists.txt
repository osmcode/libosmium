message(STATUS "Configuring unit tests...")

set(DART_TESTING_TIMEOUT 60)
include(Dart)
include_directories(include)

add_library(testlib STATIC test_main.cpp)

#----------------------------------------------------------------------

set(ALL_TESTS "")

# call with parameters:
#  TGROUP - test group (directory)
#  TNAME  - name of test
#  ARGV2  - flag to enable test (optional)
#  ARGV3  - libraries to add (optional)
function(add_unit_test TGROUP TNAME)
    set(ALL_TESTS "${ALL_TESTS};${TGROUP}/${TNAME}" PARENT_SCOPE)
    if((${ARGC} EQUAL 2) OR (ARGV2))
        if(Osmium_DEBUG)
            message("Adding test ${TGROUP}/${TNAME}")
        endif()
        add_executable(${TNAME} t/${TGROUP}/${TNAME}.cpp)
        target_link_libraries(${TNAME} testlib)
        if((ARGV2) AND (DEFINED ARGV3))
            target_link_libraries(${TNAME} ${ARGV3})
        endif()
        add_test(${TNAME} ${TNAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        set_tests_properties(${TNAME} PROPERTIES LABELS "unit;${TGROUP}")
    else()
        message("Skipped test ${TGROUP}/${TNAME} because a dependency was not found")
    endif()
endfunction()

#----------------------------------------------------------------------
#
#  Add all tests.
#
#----------------------------------------------------------------------

add_unit_test(area test_area_id)
add_unit_test(area test_node_ref_segment)

add_unit_test(basic test_box)
add_unit_test(basic test_changeset)
add_unit_test(basic test_entity_bits)
add_unit_test(basic test_location)
add_unit_test(basic test_node)
add_unit_test(basic test_node_ref)
add_unit_test(basic test_object_comparisons)
add_unit_test(basic test_relation)
add_unit_test(basic test_timestamp)
add_unit_test(basic test_way)

add_unit_test(buffer test_buffer_node)
add_unit_test(buffer test_buffer_purge)

add_unit_test(geom test_factory_with_projection
    "${GEOS_FOUND} AND ${PROJ_FOUND}"
    "${GEOS_LIBRARY};${PROJ_LIBRARY}")
add_unit_test(geom test_geojson)
add_unit_test(geom test_geos ${GEOS_FOUND} ${GEOS_LIBRARY})
add_unit_test(geom test_geos_wkb ${GEOS_FOUND} ${GEOS_LIBRARY})
add_unit_test(geom test_mercator)
add_unit_test(geom test_ogr ${GDAL_FOUND} ${GDAL_LIBRARY})
add_unit_test(geom test_projection ${PROJ_FOUND} ${PROJ_LIBRARY})
add_unit_test(geom test_wkb)
add_unit_test(geom test_wkt)

add_unit_test(index test_id_to_location ${SPARSEHASH_FOUND} "")
add_unit_test(index test_typed_mmap)

add_unit_test(io test_bzip2 ${BZIP2_FOUND} ${BZIP2_LIBRARIES})
add_unit_test(io test_file_formats)
add_unit_test(io test_reader TRUE
    "${EXPAT_LIBRARIES};${BZIP2_LIBRARIES};${ZLIB_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")

add_unit_test(tags test_filter)
add_unit_test(tags test_operators)
add_unit_test(tags test_tag_list)

add_unit_test(util test_double)
add_unit_test(util test_options)

#----------------------------------------------------------------------
#
#  Check that all tests available in test/t/*/test_*.cpp are run.
#
#----------------------------------------------------------------------

file(GLOB TESTS_IN_DIR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/t" t/*/test_*.cpp)

foreach(file ${TESTS_IN_DIR})
    string(REPLACE ".cpp" "" out1 ${file})
    string(REPLACE "//" "/" tname ${out1})
    list(FIND ALL_TESTS ${tname} found)
    if(${found} EQUAL -1)
        message(WARNING "Test '${tname}' not found in cmake config. It will not be run!")
    endif()
endforeach()

