#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - apply-changes
#
#-----------------------------------------------------------------------------

function(check_apply_changes _name _options _input _change _type _output)
    check_output(apply-changes ${_name} "apply-changes ${_options} --generator=test -f ${_type} apply-changes/${_input} apply-changes/${_change}" "apply-changes/${_output}")
endfunction()

check_apply_changes(data              ""                            input-data.osm input-change.osc "osm" output-data.osm)

add_test(NAME check-apply-changes-mixed1 COMMAND osmium apply-changes ${CMAKE_SOURCE_DIR}/test/apply-changes/input-data.osm ${CMAKE_SOURCE_DIR}/test/apply-changes/input-changes.osc -f osh)
set_tests_properties(check-apply-changes-mixed1 PROPERTIES WILL_FAIL true)

add_test(NAME check-apply-changes-mixed2 COMMAND osmium apply-changes ${CMAKE_SOURCE_DIR}/test/apply-changes/input-data.osh ${CMAKE_SOURCE_DIR}/test/apply-changes/input-changes.osc -f osm)
set_tests_properties(check-apply-changes-mixed2 PROPERTIES WILL_FAIL true)

check_apply_changes(history-osh-osh    ""               input-history.osh input-change.osc "osh" output-history.osh)
check_apply_changes(history-osh-osh-wh "--with-history" input-history.osh input-change.osc "osh" output-history.osh)
check_apply_changes(history-osm-osh-wh "--with-history" input-history.osm input-change.osc "osh" output-history.osh)
check_apply_changes(history-osh-osm-wh "--with-history" input-history.osh input-change.osc "osm" output-history.osh)

check_apply_changes(data-low "--locations-on-ways" input-data-low.osm input-change.osc "osm" output-data-low.osm)

#-----------------------------------------------------------------------------
# Test some patching features useful to redact existing OSM files
#-----------------------------------------------------------------------------
# Version 1 contains a copyright violation. The diff file contains a v2 object (with visible=false) and a modified v1 object.
check_apply_changes(redact-and-update "--redact" input-redact-and-update.osh input-redact-and-update.osc "osh" output-redact-and-update.osh)

# The input history file contains version 1 and 2, the diff wants to replace v1 by a deleted object (to redact the sensitive data in v1).
# The output history file should contain the redacted version 1 and an unmodified copy of v2.
check_apply_changes(patch-old-version "--redact" input-patch-old-version.osh input-patch-old-version.osc "osh" output-patch-old-version.osh)

# The input history file contains version 1 and 2 of an object. The diff wants to remove the metadata of v1. Version 2 should stay untouched.
check_apply_changes(redact-metadata "--redact" input-redact-metadata.osh input-redact-metadata.osc "osh" output-redact-metadata.osh)

#-----------------------------------------------------------------------------
# Test the application of diffs which have only some metadata fields on files which have only some metadata fields
#-----------------------------------------------------------------------------
# The input file has version and timestamp fields, the diff has only version fields.
check_apply_changes(version-on-version-timestamp "" input-version+timestamp.osm input-version.osc "osm" output-version-applied-on-version+timestamp.osm)
check_apply_changes(version-on-version-timestamp-low "--locations-on-ways" input-version+timestamp.osm input-version.osc "osm" output-version-applied-on-version+timestamp-low.osm)

# The input file has all metadata fields, the diff has only version fields.
check_apply_changes(version-on-all "" input-data.osm input-change-only-version.osc "osm" output-version-applied-on-all.osm)
