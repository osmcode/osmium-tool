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

function(check_renumber2 _name _in1 _in2 _out)
    set(_idxdir "${PROJECT_BINARY_DIR}/test/renumber/index")
    file(REMOVE_RECURSE ${_idxdir})
    file(MAKE_DIRECTORY ${_idxdir})
    check_output2(renumber-${_name}
                  "renumber --index-directory=${_idxdir} --generator=test -f osm --overwrite -o /dev/null renumber/${_in1}"
                  "renumber --index-directory=${_idxdir} --generator=test -f osc renumber/${_in2}"
                  "renumber/${_out}"
    )
endfunction()

check_renumber(sorted input-sorted.osm output-sorted.osm)

check_renumber2(change input-sorted.osm input-change.osc output-change.osc)

#-----------------------------------------------------------------------------
