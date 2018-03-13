#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - fileinfo
#
#-----------------------------------------------------------------------------

function(check_fileinfo _name _options _input _output)
    check_output(fileinfo ${_name} "fileinfo ${_options} fileinfo/${_input}" "fileinfo/${_output}")
endfunction()

#-----------------------------------------------------------------------------

check_fileinfo(fi1-extended "--extended" fi1.osm fi1-result.txt)

add_test(NAME fileinfo-g-generator COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g header.option.generator)
set_tests_properties(fileinfo-g-generator PROPERTIES PASS_REGULAR_EXPRESSION "^testdata\n$")

add_test(NAME fileinfo-g-unknown-option COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g header.option.foo)
set_tests_properties(fileinfo-g-unknown-option PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME fileinfo-g-fail COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g foobar)
set_tests_properties(fileinfo-g-fail PROPERTIES WILL_FAIL true)

# Test the metadata properties
add_test(NAME fileinfo-metadata-min-mixed-version-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.all_objects)
set_tests_properties(fileinfo-metadata-min-mixed-version-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^version\\+timestamp\n$")

add_test(NAME fileinfo-metadata-max-mixed-all COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects)
set_tests_properties(fileinfo-metadata-max-mixed-all PROPERTIES PASS_REGULAR_EXPRESSION "^all\n$")

add_test(NAME fileinfo-metadata-min-version-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects)
set_tests_properties(fileinfo-metadata-min-version-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^version\\+timestamp\n$")

add_test(NAME fileinfo-metadata-max-version-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects)
set_tests_properties(fileinfo-metadata-max-version-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^version\\+timestamp\n$")

#-----------------------------------------------------------------------------
