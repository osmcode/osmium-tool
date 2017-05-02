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
check_export(geojsonseq "-f geojsonseq -r" input.osm output.geojsonseq)

check_export(missing-node "-f geojson"  input-missing-node.osm output-missing-node.geojson)

check_export(error-node "-f geojson -E" input-missing-node.osm none.geojson)
set_tests_properties(export-error-node PROPERTIES WILL_FAIL true)

check_export(invalid-area "-f geojson"  input-incomplete-relation.osm output-incomplete-relation.geojson)

check_export(error-area "-f geojson -E" input-incomplete-relation.osm none.geojson)
set_tests_properties(export-error-area PROPERTIES WILL_FAIL true)

#-----------------------------------------------------------------------------
