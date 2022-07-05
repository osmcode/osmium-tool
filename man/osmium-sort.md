
# NAME

osmium-sort - sort OSM files


# SYNOPSIS

**osmium sort** \[*OPTIONS*\] *OSM-FILE*...


# DESCRIPTION

Combines and sorts the content of all input files given on the command line.

Objects are sorted by type, ID, and version. IDs are sorted negative IDs first,
then positive IDs, both ordered by their absolute values. So the sort order for
types and IDs is:

node -1, node -2, ..., node 1, node 2, ...,
way -1, way -2, ..., way 1, way 2, ...,
relation -1, relation -2, ..., relation 1, relation 2, ...

If there are several objects of the same type and with the same ID they are
ordered by ascending version. If there are several objects of the same type and
with the same ID and version the sort order is unspecified. Duplicate objects
will not be removed.

This command works with normal OSM data files, history files, and change files.

This commands reads its input file(s) only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT. (Unless the *multipass* strategy is used.)

# OPTIONS

-s, \--strategy=STRATEGY
:   Sorting strategy. The "simple" strategy reads all input files into memory,
    does the sorting and writes everything out. The "multipass" strategy reads
    the input files in three passes, one for nodes, one for ways, and one for
    relations. After reading all objects of each type, they are sorted and
    written out. This is a bit slower than the "simple" strategy, but uses
    less memory. The "multipass" strategy doesn't work when reading from STDIN.
    Default: "simple".


@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium sort** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium sort** keeps the contents of all the input files in main memory. This
will take roughly 10 times as much memory as the files take on disk in
*.osm.bz2* or *osm.pbf* format.


# EXAMPLES

Sort *in.osm.bz2* and write out to *sorted.osm.pbf*:

    osmium sort -o sorted.osm.pbf in.osm.bz2


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

