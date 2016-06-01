
# NAME

osmium-check-refs - check referential integrity of OSM file


# SYNOPSIS

**osmium check-refs** \[*OPTIONS*\] *OSM-DATA-FILE*


# DESCRIPTION

Ways in OSM files refer to OSM nodes; relations refer to nodes, ways, or other
relations. This command checks whether all objects *referenced* in the input
file are also *present* in the input file.

Referential integrity is often broken in extracts. This can lead to problems
with some uses of the OSM data. Use this command to make sure your data is
good.

If the option -r is not given, this command will only check if all nodes
referenced in ways are in the file, with the option, relations will also be
checked.

This command expects the input file to be ordered in the usual way: First
nodes in order of ID, then ways in order of ID, then relations in order of ID.

This command will only work for OSM data files, not OSM history files or
change files.


# OPTIONS

-i, --show-ids
:   Print all missing IDs to *stdout*. If you don't give this option, only a
    summary is shown.

-r, --check-relations
:   Also check referential integrity of relations. Without this option, only
    nodes in ways are checked.

@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@

# MEMORY USAGE

**osmium check-refs** will do the check in one pass through the input data. It
needs enough main memory to store all temporary data.

Largest memory need will be about 1 bit for each node ID, for a full planet
that's roughly 500 MB these days (Summer 2015). With the **-r**,
**--check-relations** option memory use will be a bit bigger.


# DIAGNOSTICS

**osmium check-refs** exits with exit code

0
  ~ if all references are satisfied

1
  ~ if there was an error processing the data or some references were not
    satisfied, or

2
  ~ if there was a problem with the command line arguments.


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)


