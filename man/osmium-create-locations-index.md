
# NAME

osmium-create-locations-index - create or update locations index from OSM file


# SYNOPSIS

**osmium create-locations-index** -i INDEX-FILE \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Create an index of all node locations from the OSM-FILE in the file INDEX-FILE.

If the INDEX-FILE exists, it will not be touched unless the **\--update/-u**
option is used.

Regardless of the size of the input file, this index will need about 8 *
highest-node-id bytes on disk. For a current planet file this is more than 50
GBytes.

The index file format is compatible to the one created by
"osmium add-location-to-ways -i dense_file_array,INDEX-FILE" and to the
flatnode store created by osm2pgsql.

When the input file is a full history file or a change file, the last location
encountered in the file for any ID ends up in the index. Usually this will be
the newest location (from the node with the highest version).

This command will not work with negative node IDs.

This commands reads its input file only once, so it can be streamed, ie. it
can read from STDIN.


# OPTIONS

-i, \--index-file=FILENAME
:   The name of the index file.

-u, \--update
:   Allow updating of existing file.

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

    osmium create-locations-index -i locations.idx planet.osm.pbf

Set a node location in the index using an input file in OPL format:

    echo "n123 x-80.6042 y28.6083" | \
        osmium create-locations-index -i locations.idx -F opl --update


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-query-locations-index**(1)](osmium-query-locations-index.html), [**osmium-file-formats**(5)](osmium-file-formats.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [osm2pgsql](https://wiki.openstreetmap.org/wiki/Osm2pgsql)

