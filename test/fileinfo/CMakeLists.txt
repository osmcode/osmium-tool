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

check_fileinfo(fi1 "--extended" fi1.osm fi1-result.txt)


#-----------------------------------------------------------------------------
