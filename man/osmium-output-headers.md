
# NAME

osmium-output-headers - Header options that can be set on output files

# DESCRIPTION

Most osmium commands that write OSM files can set values in the file header
of the OSM file using the **\--output-header** option.

The format generally is **\--output-header=OPTION=VALUE**. For some commands
you can use the special format **\--output-header=OPTION!** (ie. an exclamation
mark after the *OPTION* and no value set) to set the value to the same as in
the input file. See the individual command man pages for where this is allowed.

# HEADER OPTIONS

`generator`
:   Set the "generator program" value. Default is "osmium/VERSION". Can also
    be set using the **\--generator** option. (XML and PBF files only.)

`xml_josm_upload`
:   Value of the upload attribute on the osm XML element (true or false) for
    use in JOSM. (XML files only.)

`osmosis_replication_timestamp`
:   Timestamp used in replication (PBF files only).

`osmosis_replication_sequence_number`
:   Sequence number used in replication (PBF files only).

`osmosis_replication_base_url`
:   Base URL for change files used in replication (PBF files only).

`sorting`
:   Set the **Sort.Type_then_ID** property in the PBF header if set to
    **Type_then_ID**. Other values are currently not supported. (PBF files only).
    Note that this only sets the header option, it does not actually sort the
    file! Use **osmium sort** for that.


# EXAMPLES

Copy the file in.osm.pbf to out.osm.pbf setting the generator to **myscript**:

    osmium cat --output-header=generator=myscript -o out.osm.pbf in.osm.pbf

# SEE ALSO

* [**osmium**(1)](osmium.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [Replication headers](https://wiki.openstreetmap.org/wiki/PBF_Format)

