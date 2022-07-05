
# NAME

osmium-query-locations-index - query node locations index


# SYNOPSIS

**osmium query-locations-index** -i INDEX-FILE \[*OPTIONS*\] *NODE-ID*\
**osmium query-locations-index** -i INDEX-FILE \[*OPTIONS*\] \--dump


# DESCRIPTION

Get the location of a node from an index created with
**osmium create-locations-index** or dump the whole index into an OSM file.

The index file format is compatible to the one created by
"osmium add-location-to-ways -i dense_file_array,INDEX-FILE" and to the
flatnode store created by osm2pgsql.

This command will not work with negative node IDs.

Note that when the **\--dump** option is used, metadata (like version,
timestamp, etc.) is not written to the output file because it is all empty
anyway. Use the **\--output-format/-f** option with `add_metadata=...` to
overwrite this.


# OPTIONS

\--dump
:   Dump all node locations to an OSM file. Use the **\--output/-o** and
    **\--output-format/-f** options to set the file format to be used.
    Default is STDOUT and the OPL format, respectively.

-i, \--index-file=FILENAME
:   The name of the index file.

@MAN_COMMON_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium query-locations-index** exits with exit code

0
  ~ if everything went alright and the node location was found,

1
  ~ if the node location was not found,

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium query-locations-index** will not use a lot of memory.


# EXAMPLES

Get location of node 1234 from locations.idx:

    osmium query-locations-index -i locations.idx 1234

Dump contents of locations.idx into an OPL file:

    osmium query-locations-index -i locations.idx --dump -o nodes.opl

# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-create-locations-index**(1)](osmium-create-locations-index.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [osm2pgsql](https://wiki.openstreetmap.org/wiki/Osm2pgsql)

