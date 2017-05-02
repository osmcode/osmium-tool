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


#-----------------------------------------------------------------------------
