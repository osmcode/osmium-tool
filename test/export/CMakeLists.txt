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
check_export(geojsonseq "-f geojsonseq -x print_record_separator=false" input.osm output.geojsonseq)
check_export(spaten     "-f spaten"        input.osm output.spaten)

check_export(missing-node "-f geojson"  input-missing-node.osm output-missing-node.geojson)

check_export(error-node "-f geojson -E" input-missing-node.osm none.geojson)
set_tests_properties(export-error-node PROPERTIES WILL_FAIL true)

check_export(invalid-area "-f geojson"  input-incomplete-relation.osm output-incomplete-relation.geojson)

check_export(error-area "-f geojson -E" input-incomplete-relation.osm none.geojson)
set_tests_properties(export-error-area PROPERTIES WILL_FAIL true)

#-----------------------------------------------------------------------------

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

check_export(spaten-untagged "-f spaten --keep-untagged" input.osm output-untagged.spaten)

#-----------------------------------------------------------------------------
