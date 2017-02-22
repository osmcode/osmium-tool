#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - tags-filter
#
#-----------------------------------------------------------------------------

function(check_tags_filter _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test -f osm tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

function(check_tags_filter_i _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test -f osm -i tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

function(check_tags_filter_r _name _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test -f osm -r tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

check_tags_filter(node input.osm n/amenity output-amenity.osm)
check_tags_filter(highway input.osm w/highway output-highway.osm)
check_tags_filter(note input.osm note output-note.osm)

check_tags_filter_i(note-i input.osm note output-no-note.osm)

check_tags_filter_r(highway-r input.osm w/highway output-highway-r.osm)


#-----------------------------------------------------------------------------
