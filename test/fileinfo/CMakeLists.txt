#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - fileinfo
#
#-----------------------------------------------------------------------------

if(NOT WIN32)
    function(check_fileinfo _name _options _input _output)
        check_output(fileinfo ${_name} "fileinfo ${_options} fileinfo/${_input}" "fileinfo/${_output}")
    endfunction()

    check_fileinfo(fi1-extended "--extended --crc" fi1.osm fi1-result.txt)

    # Nlohmann JSON library before 3.9 don't have an ordered JSON type,
    # don't run tests that will not work because of that.
    if(nlohmann_json_VERSION VERSION_GREATER_EQUAL "3.9")
        check_fileinfo(fi1-extended-json "--extended --crc --json" fi1.osm fi1-result.json)
    endif()
endif()

#-----------------------------------------------------------------------------

add_test(NAME fileinfo-g-generator COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g header.option.generator)
set_tests_properties(fileinfo-g-generator PROPERTIES PASS_REGULAR_EXPRESSION "^testdata\n$")

add_test(NAME fileinfo-g-unknown-option COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g header.option.foo)
set_tests_properties(fileinfo-g-unknown-option PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME fileinfo-g-fail COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi1.osm -g foobar)
set_tests_properties(fileinfo-g-fail PROPERTIES WILL_FAIL true)


#-----------------------------------------------------------------------------
# Test the metadata properties
#-----------------------------------------------------------------------------

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.all_objects.version
add_test(NAME fileinfo-metadata-mixed-all-version COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.all_objects.version)
set_tests_properties(fileinfo-metadata-mixed-all-version PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.all_objects.timestamp
add_test(NAME fileinfo-metadata-mixed-all-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.all_objects.timestamp)
set_tests_properties(fileinfo-metadata-mixed-all-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.all_objects.changeset
add_test(NAME fileinfo-metadata-mixed-all-changeset COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.all_objects.changeset)
set_tests_properties(fileinfo-metadata-mixed-all-changeset PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.some_objects.version
add_test(NAME fileinfo-metadata-mixed-some-version COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects.version)
set_tests_properties(fileinfo-metadata-mixed-some-version PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.some_objects.timestamp
add_test(NAME fileinfo-metadata-mixed-some-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects.timestamp)
set_tests_properties(fileinfo-metadata-mixed-some-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.some_objects.changeset
add_test(NAME fileinfo-metadata-mixed-some-changeset COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects.changeset)
set_tests_properties(fileinfo-metadata-mixed-some-changeset PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.some_objects.uid
add_test(NAME fileinfo-metadata-mixed-some-uid COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects.uid)
set_tests_properties(fileinfo-metadata-mixed-some-uid PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file has objects with only version+timestamp and objects with all metadata attributes.
# searching for metadata.some_objects.user
add_test(NAME fileinfo-metadata-mixed-some-user COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi2.osm -e -g metadata.some_objects.user)
set_tests_properties(fileinfo-metadata-mixed-some-user PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.all_objects.version
add_test(NAME fileinfo-metadata-homogenous-all-version COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects.version)
set_tests_properties(fileinfo-metadata-homogenous-all-version PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.all_objects.timestamp
add_test(NAME fileinfo-metadata-homogenous-all-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects.timestamp)
set_tests_properties(fileinfo-metadata-homogenous-all-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.all_objects.changeset
add_test(NAME fileinfo-metadata-homogenous-all-changeset COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects.changeset)
set_tests_properties(fileinfo-metadata-homogenous-all-changeset PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.all_objects.uid
add_test(NAME fileinfo-metadata-homogenous-all-uid COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects.uid)
set_tests_properties(fileinfo-metadata-homogenous-all-uid PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.all_objects.user
add_test(NAME fileinfo-metadata-homogenous-all-user COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.all_objects.user)
set_tests_properties(fileinfo-metadata-homogenous-all-user PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.some_objects.version
add_test(NAME fileinfo-metadata-homogenous-some-version COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects.version)
set_tests_properties(fileinfo-metadata-homogenous-some-version PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.some_objects.timestamp
add_test(NAME fileinfo-metadata-homogenous-some-timestamp COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects.timestamp)
set_tests_properties(fileinfo-metadata-homogenous-some-timestamp PROPERTIES PASS_REGULAR_EXPRESSION "^yes\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.some_objects.changeset
add_test(NAME fileinfo-metadata-homogenous-some-changeset COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects.changeset)
set_tests_properties(fileinfo-metadata-homogenous-some-changeset PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.some_objects.uid
add_test(NAME fileinfo-metadata-homogenous-some-uid COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects.uid)
set_tests_properties(fileinfo-metadata-homogenous-some-uid PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

# The input file contains only objects with version+timestamp.
# searching for metadata.some_objects.user
add_test(NAME fileinfo-metadata-homogenous-some-user COMMAND osmium fileinfo ${CMAKE_SOURCE_DIR}/test/fileinfo/fi3.osm -e -g metadata.some_objects.user)
set_tests_properties(fileinfo-metadata-homogenous-some-user PROPERTIES PASS_REGULAR_EXPRESSION "^no\n$")

#-----------------------------------------------------------------------------
