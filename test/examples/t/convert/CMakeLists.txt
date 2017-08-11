
add_test(NAME examples_convert_help
         COMMAND osmium_convert -h)

set_tests_properties(examples_convert_help PROPERTIES
                     PASS_REGULAR_EXPRESSION "^osmium_convert .*OUTFILE")


add_test(NAME examples_convert_xml_debug
         COMMAND osmium_convert -t debug ${CMAKE_CURRENT_SOURCE_DIR}/data.osm)

set_tests_properties(examples_convert_xml_debug PROPERTIES
                     PASS_REGULAR_EXPRESSION "node 701000\n  version:   1 visible")


add_test(NAME examples_convert_xml_opl
         COMMAND osmium_convert -t opl -f xml ${CMAKE_CURRENT_SOURCE_DIR}/data.osm)

set_tests_properties(examples_convert_xml_opl PROPERTIES
                     PASS_REGULAR_EXPRESSION "n701001 v1 dV c1 t2014-01-01T00:00:00Z i1 utest T x7.11 y1.04")


add_test(NAME examples_convert_xml_pbf
         COMMAND osmium_convert -t pbf ${CMAKE_CURRENT_SOURCE_DIR}/data.osm -)


add_test(NAME examples_convert_xml_xml
         COMMAND osmium_convert -t xml ${CMAKE_CURRENT_SOURCE_DIR}/data.osm)

set_tests_properties(examples_convert_xml_xml PROPERTIES
                     PASS_REGULAR_EXPRESSION "<node id=\"701001\" ")

add_test(NAME examples_convert_osh_xml
         COMMAND osmium_convert -t xml -f osh -u ${CMAKE_CURRENT_SOURCE_DIR}/data.osm)

set_tests_properties(examples_convert_osh_xml PROPERTIES WILL_FAIL true)

