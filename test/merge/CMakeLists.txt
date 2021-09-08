#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - merge
#
#-----------------------------------------------------------------------------

function(check_merge1 _name _input1 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1}" "merge/${_output}")
endfunction()

function(check_merge2 _name _input1 _input2 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1} merge/${_input2}" "merge/${_output}")
endfunction()

function(check_merge3 _name _input1 _input2 _input3 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1} merge/${_input2} merge/${_input3}" "merge/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_merge1(i1 input1.osm output1.osm)
check_merge2(i2f input1.osm input2.osm output2.osm)
check_merge2(i2r input2.osm input1.osm output2.osm)
check_merge3(i3f input1.osm input2.osm input3.osm output3.osm)

# If both input files do not have timestamp attributes
check_merge2(i2f-only-version input1-only-version.osm input2-only-version.osm output2-12-only-version.osm)
check_merge2(i2r-only-version input2-only-version.osm input1-only-version.osm output2-12-only-version.osm)

do_test(merge-unsorted-types-wn "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-types-wn.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "must be nodes, then ways, then relations")
do_test(merge-unsorted-types-nrw "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-types-nrw.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "must be nodes, then ways, then relations")
do_test(merge-unsorted-ids-n "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-ids-n.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "smaller ids must come first")
do_test(merge-unsorted-ids-w "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-ids-w.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "smaller ids must come first")
do_test(merge-unsorted-versions-21 "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-versions-21.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "smaller version must come first")
do_test(merge-unsorted-versions-132 "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-versions-132.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "smaller version must come first")
do_test(merge-unsorted-versions-133 "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/unsorted-versions-133.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "with same version.")

do_test(merge-same-ids-warning "osmium merge ${PROJECT_SOURCE_DIR}/test/merge/same-ids.osm ${PROJECT_SOURCE_DIR}/test/merge/empty.osm -f opl" "Multiple objects with same id")
check_output(merge merge-same-ids-h "merge --generator=test --with-history -f osm merge/same-ids.osm merge/empty.osm" "merge/output-same-ids.osm")

#-----------------------------------------------------------------------------
