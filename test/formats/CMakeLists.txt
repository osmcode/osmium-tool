#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - formats
#
#-----------------------------------------------------------------------------

function(check_formats _name _input _format _output)
    check_output(formats ${_name} "cat --generator=test --output-header xml_josm_upload=false -f ${_format} ${PROJECT_SOURCE_DIR}/test/formats/${_input}" "formats/${_output}")
endfunction()

#-----------------------------------------------------------------------------

# empty file (no objects)
check_formats(emptyxx   empty.osm xml empty.osm)
check_formats(emptyxo   empty.osm opl empty.osm.opl)
check_formats(emptyxp   empty.osm pbf                                              empty.osm.pbf)
check_formats(emptyxpd  empty.osm pbf,pbf_dense_nodes=false                        empty-nodensenodes.osm.pbf)
check_formats(emptyxpc  empty.osm pbf,pbf_compression=none                         empty-nocompression.osm.pbf)
check_formats(emptyxpm  empty.osm pbf,add_metadata=false                           empty-nometadata.osm.pbf)
check_formats(emptyxpdm empty.osm pbf,pbf_dense_nodes=false,add_metadata=false     empty-nodensenodes-nometadata.osm.pbf)

check_formats(emptypx empty.osm.pbf xml empty.osm)
check_formats(emptypo empty.osm.pbf opl empty.osm.opl)
check_formats(emptypp empty.osm.pbf pbf empty.osm.pbf)

check_formats(emptypxd empty-nodensenodes.osm.pbf xml empty.osm)
check_formats(emptypod empty-nodensenodes.osm.pbf opl empty.osm.opl)
check_formats(emptyppd empty-nodensenodes.osm.pbf pbf empty.osm.pbf)

check_formats(emptypxc empty-nocompression.osm.pbf xml empty.osm)
check_formats(emptypoc empty-nocompression.osm.pbf opl empty.osm.opl)
check_formats(emptyppc empty-nocompression.osm.pbf pbf empty.osm.pbf)

# normal file
check_formats(f1xx   f1.osm xml f1.osm)
check_formats(f1xo   f1.osm opl f1.osm.opl)

if(RUN_TESTS_WITH_BINARY_COMPARE)
    check_formats(f1xp   f1.osm pbf                                          f1.osm.pbf)
    check_formats(f1xpd  f1.osm pbf,pbf_dense_nodes=false                    f1-nodensenodes.osm.pbf)
    check_formats(f1xpm  f1.osm pbf,add_metadata=false                       f1-nometadata.osm.pbf)
    check_formats(f1xpdm f1.osm pbf,pbf_dense_nodes=false,add_metadata=false f1-nodensenodes-nometadata.osm.pbf)
endif()

check_formats(f1xpc f1.osm pbf,pbf_compression=none f1-nocompression.osm.pbf)

check_formats(f1px f1.osm.pbf xml f1.osm)
check_formats(f1po f1.osm.pbf opl f1.osm.opl)

check_formats(f1pxd f1-nodensenodes.osm.pbf xml f1.osm)
check_formats(f1pod f1-nodensenodes.osm.pbf opl f1.osm.opl)

check_formats(f1pxc f1-nocompression.osm.pbf xml f1.osm)
check_formats(f1poc f1-nocompression.osm.pbf opl f1.osm.opl)

if(RUN_TESTS_WITH_BINARY_COMPARE)
    check_formats(f1pp f1.osm.pbf pbf f1.osm.pbf)
    check_formats(f1ppd f1-nodensenodes.osm.pbf pbf f1.osm.pbf)
    check_formats(f1ppc f1-nocompression.osm.pbf pbf f1.osm.pbf)
endif()

#-----------------------------------------------------------------------------
