#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - changeset-filter
#
#-----------------------------------------------------------------------------

function(check_changeset_filter _name _options _input _output)
    check_output(changeset-filter ${_name} "changeset-filter --generator=test -f osm ${_options} changeset-filter/${_input}" "changeset-filter/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_changeset_filter(cf1-no-option          ""                     input1.osm output1-all.osm)
check_changeset_filter(cf1-with-discussion    "--with-discussion"    input1.osm output-empty.osm)
check_changeset_filter(cf1-without-discussion "--without-discussion" input1.osm output1-all.osm)
check_changeset_filter(cf1-with-changes       "--with-changes"       input1.osm output1-first.osm)
check_changeset_filter(cf1-without-changes    "--without-changes"    input1.osm output1-second.osm)
check_changeset_filter(cf1-open               "--open"               input1.osm output-empty.osm)
check_changeset_filter(cf1-closed             "--closed"             input1.osm output1-all.osm)
check_changeset_filter(cf1-user               "--user=Elbert"        input1.osm output1-first.osm)
check_changeset_filter(cf1-uid                "--uid=1233268"        input1.osm output1-second.osm)

check_changeset_filter(cfe-open   "--open"   input-open.osm output-open.osm)
check_changeset_filter(cfe-closed "--closed" input-open.osm output-empty.osm)

check_changeset_filter(cf1-after01 "--after=2013-03-22T02:08:50Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after02 "--after=2013-03-22T02:08:54Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after03 "--after=2013-03-22T02:08:55Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after04 "--after=2013-03-22T02:08:56Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after05 "--after=2013-03-22T02:08:57Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after06 "--after=2013-03-22T02:08:58Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-after07 "--after=2013-03-22T02:08:59Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after08 "--after=2013-03-22T02:09:00Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after09 "--after=2013-03-22T02:09:10Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after10 "--after=2013-03-22T02:09:11Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after11 "--after=2013-03-22T02:09:12Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after12 "--after=2013-03-22T03:09:10Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after13 "--after=2013-03-22T03:09:11Z" input1.osm output1-second.osm)
check_changeset_filter(cf1-after14 "--after=2013-03-22T03:09:12Z" input1.osm output-empty.osm)
check_changeset_filter(cf1-after15 "--after=2013-03-22T03:09:20Z" input1.osm output-empty.osm)

check_changeset_filter(cfe-after01 "--after=2013-03-22T04:20:24Z" input-open.osm output-open.osm)
check_changeset_filter(cfe-after02 "--after=2013-03-22T04:20:25Z" input-open.osm output-open.osm)
check_changeset_filter(cfe-after03 "--after=2013-03-22T04:20:26Z" input-open.osm output-open.osm)

check_changeset_filter(cf1-before01 "--before=2013-03-22T02:08:50Z" input1.osm output-empty.osm)
check_changeset_filter(cf1-before02 "--before=2013-03-22T02:08:54Z" input1.osm output-empty.osm)
check_changeset_filter(cf1-before03 "--before=2013-03-22T02:08:55Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before04 "--before=2013-03-22T02:08:56Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before05 "--before=2013-03-22T02:08:57Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before06 "--before=2013-03-22T02:08:58Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before07 "--before=2013-03-22T02:08:59Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before08 "--before=2013-03-22T02:09:00Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before09 "--before=2013-03-22T02:09:10Z" input1.osm output1-first.osm)
check_changeset_filter(cf1-before10 "--before=2013-03-22T02:09:11Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-before11 "--before=2013-03-22T02:09:12Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-before12 "--before=2013-03-22T03:09:10Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-before13 "--before=2013-03-22T03:09:11Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-before14 "--before=2013-03-22T03:09:12Z" input1.osm output1-all.osm)
check_changeset_filter(cf1-before15 "--before=2013-03-22T03:09:20Z" input1.osm output1-all.osm)

check_changeset_filter(cfe-before01 "--before=2013-03-22T04:20:24Z" input-open.osm output-empty.osm)
check_changeset_filter(cfe-before02 "--before=2013-03-22T04:20:25Z" input-open.osm output-open.osm)
check_changeset_filter(cfe-before03 "--before=2013-03-22T04:20:26Z" input-open.osm output-open.osm)

check_changeset_filter(cf1-bbox01 "--bbox 120,-11,121,-10" input1.osm output1-first.osm)
check_changeset_filter(cf1-bbox02 "--bbox 130,-31,131,-30" input1.osm output-empty.osm)

#-----------------------------------------------------------------------------
