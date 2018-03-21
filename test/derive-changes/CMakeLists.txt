#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - derive-changes
#
#-----------------------------------------------------------------------------

function(check_derive_changes _name _options _input1 _input2 _output)
    check_output(derive-changes ${_name} "derive-changes --generator=test -f osc ${_options} derive-changes/${_input1} derive-changes/${_input2}" "derive-changes/${_output}")
endfunction()

check_derive_changes(normal       ""                    input1.osm input2.osm output.osc)
check_derive_changes(keep_details "--keep-details"      input1.osm input2.osm output-keep-details.osc)
check_derive_changes(incr_version "--increment-version" input1.osm input2.osm output-incr-version.osc)

# Tests with input file which don't have the same amount of metadata fields:
# File 1 has all metadata fields, file 2 has only version fields.
check_derive_changes(new_file_only_versions "" input1.osm input2-only-versions.osm output-2-only-version.osc)

# File 1 has only version and timestamp fields, file 2 has only version fields.
check_derive_changes(version_timestamp_with_version "" input1-only-version-timestamp.osm input2-only-versions.osm output-2-only-version-timestamp.osc)

# File 1 has only version fields, file 2 has version and timestamp fields.
check_derive_changes(version_with_version_timestamp "" input1-only-version.osm input2-only-version-timestamp.osm output-2-version-with-version-timestamp.osc)

# File 1 has only version fields, file 2 has all metadata fields.
check_derive_changes(version_with_all "" input1-only-version.osm input2-all-with-relation.osm output-2-version-with-all.osc)

#-----------------------------------------------------------------------------
