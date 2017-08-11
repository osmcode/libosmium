
add_test(NAME examples_index_lookup_help
         COMMAND osmium_index_lookup -h)

set_tests_properties(examples_index_lookup_help PROPERTIES
                     PASS_REGULAR_EXPRESSION "^Usage: osmium_index_lookup")


add_test(NAME examples_index_lookup_unknown_file
         COMMAND osmium_index_lookup --list=file_does_not_exist --type=location --dump)

set_tests_properties(examples_index_lookup_unknown_file PROPERTIES
                     WILL_FAIL true)

