#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - add-locations-to-ways
#
#-----------------------------------------------------------------------------

function(check_add_locations_to_ways _name _options _input _output)
    check_output(add-locations-to-ways ${_name} "add-locations-to-ways ${_options} --generator=test --output-format=xml add-locations-to-ways/${_input}" "add-locations-to-ways/${_output}")
endfunction()

check_add_locations_to_ways(taggednodes "" input.osm output.osm)
check_add_locations_to_ways(allnodes "-n" input.osm output-n.osm)


#-----------------------------------------------------------------------------
