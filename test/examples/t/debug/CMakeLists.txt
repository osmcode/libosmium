
add_test(NAME examples_debug
         COMMAND osmium_debug ${CMAKE_CURRENT_SOURCE_DIR}/data.osm)

set_tests_properties(examples_debug PROPERTIES
                     PASS_REGULAR_EXPRESSION "  id=1\n.*  id=2\n.*  id=10\n.*  id=20\n"
)

