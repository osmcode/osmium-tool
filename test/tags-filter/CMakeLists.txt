#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - tags-filter
#
#-----------------------------------------------------------------------------

function(check_tags_filter _name _flags _input _expressions _output)
    check_output(tags-filter ${_name} "tags-filter --generator=test --output-header=xml_josm_upload=false -f osm ${_flags} tags-filter/${_input} ${_expressions}" "tags-filter/${_output}")
endfunction()

check_tags_filter(node        ""      input.osm n/amenity output-amenity.osm)
check_tags_filter(node-R      "-R"    input.osm n/amenity output-amenity.osm)

check_tags_filter(highway-R   "-R"    input.osm w/highway output-highway-R.osm)
check_tags_filter(note-R      "-R"    input.osm note output-note-R.osm)

check_tags_filter(note-iR     "-i -R" input.osm note output-note-iR.osm)

check_tags_filter(highway     ""      input.osm w/highway output-highway.osm)

check_tags_filter(highway-i   "-i"    input.osm w/highway output-highway-i.osm)

check_tags_filter(highway-n-i "-i"    input-nodes.osm w/highway output-nodes-highway-i.osm)

check_tags_filter(note-rel    ""      input.osm r/note output-note-rel.osm)

check_tags_filter(highway-t   "-t"    input.osm w/highway output-highway-t.osm)
check_tags_filter(highway-it  "-i -t" input.osm w/highway output-highway-it.osm)
check_tags_filter(note-rel-t  "-t"    input.osm r/note output-note-rel-t.osm)


check_tags_filter(site-1     ""   input-site.osm r/site=1 output-site-1.osm)
check_tags_filter(site-2     ""   input-site.osm r/site=2 output-site-2.osm)
check_tags_filter(site-R     "-R" input-site.osm r/site=2 output-site-R.osm)
check_tags_filter(site-R-all "-R" input-site.osm r/site output-site-R-all.osm)
check_tags_filter(site-t     "-t" input-site.osm r/site output-site-t.osm)

#-----------------------------------------------------------------------------
