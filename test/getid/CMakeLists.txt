#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - getid
#
#-----------------------------------------------------------------------------

function(check_getid _name _input _output)
    check_output(getid ${_name} "getid --generator=test --output-header=xml_josm_upload=false -f osm getid/${_input} n11,n12 w21" "getid/${_output}")
endfunction()

function(check_getid_file _name _file _input _output)
    check_output(getid ${_name} "getid --generator=test --output-header=xml_josm_upload=false -i getid/${_file} -f osm getid/${_input}" "getid/${_output}")
endfunction()

check_getid(n input.osm output.osm)
check_getid_file(file1 idfile input.osm output-file.osm)

#-----------------------------------------------------------------------------

function(check_getid_r _name _source _input _output)
    check_output(getid ${_name}   "getid -r --generator=test -f osm getid/${_source}.osm -I getid/${_input}.osm" "getid/${_output}.osm")
    check_output(getid ${_name}-i "getid -r --generator=test -f osm getid/${_source}.osm -i getid/${_input}.id"  "getid/${_output}.osm")
endfunction()

function(check_getid_r_fail _name _input)
    check_output(getid ${_name}   "getid -r --generator=test -f osm getid/source.osm -I getid/${_input}.osm" "getid/out-empty.osm" 1)
    check_output(getid ${_name}-i "getid -r --generator=test -f osm getid/source.osm -i getid/${_input}.id"  "getid/out-empty.osm" 1)
endfunction()

check_getid_r(n10 source in10 out10)
check_getid_r(w21 source in21 out21)
check_getid_r(r30 source in30 out30)
check_getid_r(r31 source in31 out31)
check_getid_r(r32 source in32 out32)

check_getid_r(n10nrr source-no-rr in10 out10)
check_getid_r(w21nrr source-no-rr in21 out21)
check_getid_r(r30nrr source-no-rr in30 out30)
check_getid_r(r32nrr source-no-rr in32 out32)

check_getid_r_fail(missing-n19 in19)
check_getid_r_fail(missing-w29 in29)
check_getid_r_fail(missing-r39 in39)

check_getid_r(relloop relloop relloop relloop-out)


#-----------------------------------------------------------------------------
