#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - add-refs
#
#-----------------------------------------------------------------------------

function(check_add_refs _name _source _input _output)
    check_output(add-refs ${_name} "add-refs --generator=test -f osm -s add-refs/${_source}.osm add-refs/${_input}.osm" "add-refs/${_output}.osm")
    check_output(add-refs ${_name}-i "add-refs --generator=test -f osm -s add-refs/${_source}.osm -i add-refs/${_input}.id" "add-refs/${_output}.osm")
endfunction()

check_add_refs(n10 source in10 out10)
check_add_refs(w21 source in21 out21)
check_add_refs(r30 source in30 out30)
check_add_refs(r31 source in31 out31)
check_add_refs(r32 source in32 out32)

check_add_refs(n10nrr source-no-rr in10 out10)
check_add_refs(w21nrr source-no-rr in21 out21)
check_add_refs(r30nrr source-no-rr in30 out30)
check_add_refs(r32nrr source-no-rr in32 out32)

check_add_refs(missing-n19 source in19 out31)
set_tests_properties(add-refs-missing-n19 PROPERTIES WILL_FAIL true)
set_tests_properties(add-refs-missing-n19-i PROPERTIES WILL_FAIL true)

check_add_refs(missing-w29 source in29 out31)
set_tests_properties(add-refs-missing-w29 PROPERTIES WILL_FAIL true)
set_tests_properties(add-refs-missing-w29-i PROPERTIES WILL_FAIL true)

check_add_refs(missing-r39 source in39 out31)
set_tests_properties(add-refs-missing-r39 PROPERTIES WILL_FAIL true)
set_tests_properties(add-refs-missing-r39-i PROPERTIES WILL_FAIL true)

check_add_refs(relloop relloop relloop relloop-out)


#-----------------------------------------------------------------------------
