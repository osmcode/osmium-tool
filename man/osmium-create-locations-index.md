
# NAME

osmium-create-locations-index - create locations index from OSM file


# SYNOPSIS

**osmium create-locations-index** -i INDEX-FILE \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Create an index of all node locations from the OSM-FILE in the file INDEX-FILE.
This index will need about 8 * highest-node-id bytes on disk. For a current
planet file this is more than 50 GBytes.

If the INDEX-FILE exists, it will be updated.

The index file format is compatible to the one created by
"osmium add-location-to-ways -i dense_file_array,INDEX-FILE" and to the
flatnode store created by osm2pgsql.

This command will not work on full history files.

This commands reads its input file only once, so it can be streamed, ie. it
can read from STDIN.


# OPTIONS

-i, \--index-file=FILENAME
:   The name of the index file.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@

# DIAGNOSTICS

**osmium create-locations-index** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium create-locations-index** will not use a lot of memory.


# EXAMPLES

Create node locations index from planet:

    osmium create-locations-index -i index.dat planet.osm.pbf


# SEE ALSO

* **osmium**(1), **osmium-query-locations-index**(1), **osmium-file-formats**(5)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [osm2pgsql](https://wiki.openstreetmap.org/wiki/Osm2pgsql)

