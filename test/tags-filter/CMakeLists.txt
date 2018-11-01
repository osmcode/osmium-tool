#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - tags-filter
#
#-----------------------------------------------------------------------------

function(check_tags_filter_R _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test --output-header=xml_josm_upload=false -f osm -R tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

function(check_tags_filter_i _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test --output-header=xml_josm_upload=false -f osm -i -R tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

function(check_tags_filter _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test --output-header=xml_josm_upload=false -f osm tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

check_tags_filter_R(node input.osm n/amenity output-amenity.osm)
check_tags_filter_R(highway input.osm w/highway output-highway.osm)
check_tags_filter_R(note input.osm note output-note.osm)

check_tags_filter_i(note-i input.osm note output-no-note.osm)

check_tags_filter(highway-r input.osm w/highway output-highway-r.osm)


#-----------------------------------------------------------------------------
