#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - renumber
#
#-----------------------------------------------------------------------------

function(check_renumber _name _input _output)
    check_output(renumber-${_name} "renumber --generator=test -f osm renumber/${_input}" "renumber/${_output}")
endfunction()

check_renumber(sorted input-sorted.osm output-sorted.osm)

#-----------------------------------------------------------------------------
