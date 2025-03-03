#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - export
#
#-----------------------------------------------------------------------------

function(check_export _name _options _input _output)
    check_output(export ${_name} "export ${_options} export/${_input}" "export/${_output}")
endfunction()

check_export(geojson    "-f geojson"       input.osm output.geojson)
check_export(geojsonmp  "-f geojson -u type_id" input-mp.osm output-mp.geojson)
check_export(geojsonseq "-f geojsonseq -x print_record_separator=false" input.osm output.geojsonseq)
check_export(geojsonuid "-f geojsonseq -u type_id" input.osm output-uid.geojsonseq)
check_export(geojsoncnt "-f geojsonseq -u counter" input.osm output-cnt.geojsonseq)
check_export(geojsonattr "-f geojson -n -a type,id,version,changeset,timestamp,uid,user,way_nodes" input.osm output-attr.geojson)
check_export(geojsonchar "-f geojson -n -a type,id,version,changeset,timestamp,uid,user,way_nodes" input-chars.osm output-chars.geojson)

check_export(pg         "-f pg"            input.osm output.pg)

check_export(missing-node "-f geojson" input-missing-node.osm output-missing-node.geojson)
check_export(single-node-way "-f geojson" input-single-node-way.osm output-empty.geojson)

add_test(NAME export-error-node COMMAND osmium export -f geojson -E ${CMAKE_SOURCE_DIR}/test/export/input-missing-node.osm)
set_tests_properties(export-error-node PROPERTIES WILL_FAIL true)

add_test(NAME export-error-single-node-way COMMAND osmium export -f geojson -E ${CMAKE_SOURCE_DIR}/test/export/input-single-node-way.osm)
set_tests_properties(export-error-single-node-way PROPERTIES WILL_FAIL true)

check_export(invalid-area "-f geojson"  input-incomplete-relation.osm output-incomplete-relation.geojson)

add_test(NAME export-error-area COMMAND osmium export -f geojson -E ${CMAKE_SOURCE_DIR}/test/export/input-incomplete-relation.osm)
set_tests_properties(export-error-area PROPERTIES WILL_FAIL true)

add_test(NAME export-error-incomplete-rel COMMAND osmium export -f geojson -E ${CMAKE_SOURCE_DIR}/test/export/input-incomplete-rel-missing-way.osm)
set_tests_properties(export-error-incomplete-rel PROPERTIES WILL_FAIL true)

#-----------------------------------------------------------------------------

check_export(attributes  "-E -f text -a id" way.osm way-all.txt)

check_export(c-empty-empty-n "-E -f text --keep-untagged -c export/config-empty-empty.json" way.osm way-all-n.txt)
set_tests_properties(export-c-empty-empty-n PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)
check_export(c-empty-empty   "-E -f text                 -c export/config-empty-empty.json" way.osm way-all.txt)
set_tests_properties(export-c-empty-empty PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)

check_export(c-null-null  "-E -f text -c export/config-null-null.json" way.osm way-all.txt)
check_export(c-undefined  "-E -f text -c export/config-undefined.json" way.osm way-all.txt)

check_export(c-tag-empty  "-E -f text -c export/config-tag-empty.json" way.osm way-tag-empty.txt)
set_tests_properties(export-c-tag-empty PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)
check_export(c-empty-tag  "-E -f text -c export/config-empty-tag.json" way.osm way-empty-tag.txt)
set_tests_properties(export-c-empty-tag PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)
check_export(c-tag-tag    "-E -f text -c export/config-tag-tag.json"   way.osm way-tag-tag.txt)

check_export(c-tagx-empty "-E -f text -c export/config-tagx-empty.json" way.osm way-tagx-empty.txt)
set_tests_properties(export-c-tagx-empty PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)
check_export(c-empty-tagx "-E -f text -c export/config-empty-tagx.json" way.osm way-empty-tagx.txt)
set_tests_properties(export-c-empty-tagx PROPERTIES ENVIRONMENT osmium_cmake_stderr=ignore)
check_export(c-tagx-tagx  "-E -f text -c export/config-tagx-tagx.json"  way.osm way-tagx-tagx.txt)

check_export(c-true-true   "-E -f text -c export/config-true-true.json"   way.osm way-all.txt)
check_export(c-false-false "-E -f text -c export/config-false-false.json" way.osm way-none.txt)

check_export(c-null-tag    "-E -f text -c export/config-null-tag.json" way.osm way-null-tag.txt)
check_export(c-tag-null    "-E -f text -c export/config-tag-null.json" way.osm way-tag-null.txt)

#-----------------------------------------------------------------------------

check_export(pg-untagged "-f pg --keep-untagged" input.osm output-untagged.pg)

#-----------------------------------------------------------------------------
