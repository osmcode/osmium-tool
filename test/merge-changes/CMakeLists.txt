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

# Both input files have all metadata attributes
check_merge_changes(merged "" change1.osc change2.osc merged.osc)
check_merge_changes(simplified "--simplify" change1.osc change2.osc simplified.osc)

# Both input files have only version attributes
check_merge_changes(merged-both-version "" change1-only-version.osc change2-only-version.osc merged-both-only-version.osc)
check_merge_changes(simplified-both-version "--simplify" change1-only-version.osc change2-only-version.osc simplified-both-only-version.osc)

# The first input file has only version attributes
check_merge_changes(merged-first-only-version "" change1-only-version.osc change2.osc merged-first-only-version.osc)
check_merge_changes(simplified-first-only-version "--simplify" change1-only-version.osc change2.osc simplified-first-only-version.osc)

# The second input file has only version attributes
check_merge_changes(merged-second-only-version "" change1.osc change2-only-version.osc merged-second-only-version.osc)
check_merge_changes(simplified-second-only-version "--simplify" change1.osc change2-only-version.osc simplified-second-only-version.osc)

#-----------------------------------------------------------------------------
