#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - time-filter
#
#-----------------------------------------------------------------------------

function(check_time_filter _name _type _from _to _output)
    check_output(time-filter ${_name} "time-filter --generator=test --output-header=xml_josm_upload=false -f ${_type} time-filter/input.osh ${_from} ${_to}" "time-filter/output-${_output}.${_type}")
endfunction()

#-----------------------------------------------------------------------------

check_time_filter(last osm ""                   "" last)
check_time_filter(ts1  osm 2015-01-01T01:00:00Z "" ts1)
check_time_filter(ts1a osm 2015-01-01T01:01:00Z "" ts1)
check_time_filter(ts2  osm 2015-01-01T02:00:00Z "" ts2)
check_time_filter(ts2a osm 2015-01-01T02:01:00Z "" ts2)
check_time_filter(ts3  osm 2015-01-01T03:00:00Z "" ts3)
check_time_filter(ts3a osm 2015-01-01T03:01:00Z "" ts3)

check_time_filter(range-0-5   osh 2015-01-01T00:00:00Z 2015-01-01T05:00:00Z range-0-5)
check_time_filter(range-1-2   osh 2015-01-01T01:00:00Z 2015-01-01T02:00:00Z range-1-2)
check_time_filter(range-1a-2a osh 2015-01-01T01:01:00Z 2015-01-01T02:01:00Z range-1a-2a)
check_time_filter(range-2-3   osh 2015-01-01T02:00:00Z 2015-01-01T03:00:00Z range-2-3)
check_time_filter(range-2-3a  osh 2015-01-01T02:00:00Z 2015-01-01T03:01:00Z range-2-3a)
check_time_filter(range-2-4   osh 2015-01-01T02:00:00Z 2015-01-01T04:00:00Z range-2-4)


#-----------------------------------------------------------------------------
