#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - merge-changes
#
#-----------------------------------------------------------------------------

function(check_merge_changes _name _options _in1 _in2 _output)
    check_output(merge-changes ${_name} "merge-changes ${_options} --generator=test -f osc merge-changes/${_in1} merge-changes/${_in2}" "merge-changes/${_output}")
endfunction()

check_merge_changes(merged "" change1.osc change2.osc merged.osc)
check_merge_changes(simplified "--simplify" change1.osc change2.osc simplified.osc)


#-----------------------------------------------------------------------------
