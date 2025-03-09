#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - extract
#
#-----------------------------------------------------------------------------

function(check_extract _name _input _output _opts)
    check_output(extract ${_name} "extract --generator=test -f osm extract/${_input} ${_opts} -b 0,0,1.5,10" "extract/${_output}")
endfunction()

function(check_extract_cfg _name _input _output _opts)
    check_output(extract cfg_${_name} "extract --generator=test extract/${_input} ${_opts} -c ${CMAKE_CURRENT_SOURCE_DIR}/config.json" "extract/${_output}")
endfunction()

function(check_extract_opl _name _input _output _opts)
    check_output(extract ${_name} "extract --generator=test -f opl extract/${_input} ${_opts}" "extract/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_extract(clean                input1.osm output-clean.osm "--clean version --clean uid")

check_extract(simple               input1.osm output-simple.osm "-s simple --output-header=xml_josm_upload!")
check_extract(complete_ways        input1.osm output-complete-ways.osm "-s complete_ways")
check_extract(complete_ways_norels input1.osm output-complete-ways-norels.osm "-s complete_ways -S relations=false")
check_extract(smart_default        input1.osm output-smart.osm "-s smart")
check_extract(smart_mp             input1.osm output-smart.osm "-s smart -S types=multipolygon")
check_extract(smart_any            input1.osm output-smart.osm "-s smart -S types=any")
check_extract(smart_nonmp          input1.osm output-smart-nonmp.osm "-s smart -S types=x")

check_extract_cfg(simple           input1.osm output-simple.osm "-s simple --output-header=xml_josm_upload=false")

#-----------------------------------------------------------------------------

check_extract(clean64                input64.osm output-clean64.osm "--clean version --clean uid")

check_extract(simple64               input64.osm output-simple64.osm "-s simple --output-header=xml_josm_upload!")
check_extract(complete_ways64        input64.osm output-complete-ways64.osm "-s complete_ways")
check_extract(complete_ways_norels64 input64.osm output-complete-ways-norels64.osm "-s complete_ways -S relations=false")
check_extract(smart_default64        input64.osm output-smart64.osm "-s smart")
check_extract(smart_mp64             input64.osm output-smart64.osm "-s smart -S types=multipolygon")
check_extract(smart_any64            input64.osm output-smart64.osm "-s smart -S types=any")
check_extract(smart_nonmp64          input64.osm output-smart-nonmp64.osm "-s smart -S types=x")

check_extract_cfg(simple64           input64.osm output-simple64.osm "-s simple --output-header=xml_josm_upload=false")

#-----------------------------------------------------------------------------

check_extract_opl(antimeridian-east-bbox antimeridian.opl output-antimeridian-east.opl "--bbox=160,60,180,80")
check_extract_opl(antimeridian-west-bbox antimeridian.opl output-antimeridian-west.opl "--bbox=-180,60,-160,80")
check_extract_opl(antimeridian-east-poly antimeridian.opl output-antimeridian-east.opl "--polygon=extract/polygon-russia-east.geojson")
check_extract_opl(antimeridian-west-poly antimeridian.opl output-antimeridian-west.opl "--polygon=extract/polygon-russia-west.geojson")
check_extract_opl(antimeridian-both-poly antimeridian.opl output-antimeridian-both.opl "--polygon=extract/polygon-russia-all.geojson")
check_extract_opl(antimeridian-reverse-poly antimeridian.opl output-antimeridian-both.opl "--polygon=extract/polygon-russia-reverse.geojson")

check_extract_opl(antimeridian-alaska-east-json w42394837.osm w42394837.opl "--polygon=extract/polygon-us-alaska.geojson")
check_extract_opl(antimeridian-alaska-west-json w46113981.osm w46113981.opl "--polygon=extract/polygon-us-alaska.geojson")
check_extract_opl(antimeridian-alaska-east-poly w42394837.osm w42394837.opl "--polygon=extract/polygon-us-alaska.poly")
check_extract_opl(antimeridian-alaska-west-poly w46113981.osm w46113981.opl "--polygon=extract/polygon-us-alaska.poly")

check_extract_opl(antimeridian-alaska-east-json-no-feature w42394837.osm w42394837.opl "--polygon=extract/polygon-us-alaska-no-feature.geojson")

#-----------------------------------------------------------------------------
