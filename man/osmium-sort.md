
# NAME

osmium-sort - sort OSM files


# SYNOPSIS

**osmium sort** \[*OPTIONS*\] *OSM-FILE*...


# DESCRIPTION

Merges the content of all input files given on the command line and sort the
result. Objects are sorted by type, ID, and version.

This works with normal OSM data files, history files, and change files.

@MAN_COMMON_OPTIONS@
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

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

