#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - fileinfo
#
#-----------------------------------------------------------------------------

function(check_fileinfo _name _options _input _output)
    check_output(fileinfo ${_name} "fileinfo ${_options} fileinfo/${_input}" "fileinfo/${_output}")
endfunction()

#-----------------------------------------------------------------------------

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

if(${BIG_ENDIAN})
    check_fileinfo(fi1 "--extended" fi1.osm fi1-result-be.txt)
else(${BIG_ENDIAN})
    check_fileinfo(fi1 "--extended" fi1.osm fi1-result.txt)
endif(${BIG_ENDIAN})

#-----------------------------------------------------------------------------
