#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - removeid
#
#-----------------------------------------------------------------------------

function(check_removeid _name _input _ids _output)
    check_output(removeid ${_name} "removeid --generator=test --output-header=xml_josm_upload=false -f osm removeid/${_input} ${_ids}" "removeid/${_output}")
endfunction()

function(check_removeid_file _name _file _input _output)
    check_output(removeid ${_name} "removeid --generator=test --output-header=xml_josm_upload=false -i removeid/${_file} -f osm removeid/${_input}" "removeid/${_output}")
endfunction()

check_removeid(n11n12 input.osm n11,n12 output-n11n12.osm)
check_removeid(n10w21 input.osm 10,w21 output-n10w21.osm)
check_removeid(r30 input.osm r30 output-r30.osm)

check_removeid_file(n12w20 n12w20.ids input.osm output-n12w20.osm)

#-----------------------------------------------------------------------------
