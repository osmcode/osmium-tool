#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - tags-filter
#
#-----------------------------------------------------------------------------

function(check_tags_filter _name _flags _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test --output-header=xml_josm_upload=false -f osm ${_flags} tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

check_tags_filter(node-R    "-R"    input.osm n/amenity output-amenity.osm)
check_tags_filter(highway-R "-R"    input.osm w/highway output-highway.osm)
check_tags_filter(note      "-R"    input.osm note output-note.osm)

check_tags_filter(note-i-R  "-i -R" input.osm note output-no-note.osm)

check_tags_filter(highway   ""      input.osm w/highway output-highway-r.osm)


#-----------------------------------------------------------------------------
