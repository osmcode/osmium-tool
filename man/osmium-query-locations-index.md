
# NAME

osmium-query-locations-index - query node locations index


# SYNOPSIS

**osmium query-locations-index** -i INDEX-FILE \[*OPTIONS*\] *NODE-ID*


# DESCRIPTION

Get the location of a node from an index created with
**osmium create-locations-index**. This command is mostly intended for
debugging.

The index file format is compatible to the one created by
"osmium add-location-to-ways -i dense_file_array,INDEX-FILE" and to the
flatnode store created by osm2pgsql.

This command will not work with negative node IDs.


# OPTIONS

-i, \--index-file=FILENAME
:   The name of the index file.

@MAN_COMMON_OPTIONS@

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

Get location of node 1234 from index.dat:

    osmium query-locations-index -i index.dat 1234


# SEE ALSO

* **osmium**(1), **osmium-create-locations-index**(1), **osmium-file-formats**(5)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [osm2pgsql](https://wiki.openstreetmap.org/wiki/Osm2pgsql)

