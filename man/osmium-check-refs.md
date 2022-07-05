
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

If the option **\--check-relations/-r** is not given, this command will only
check if all nodes referenced in ways are in the file, with the option,
relations will also be checked.

This command expects the input file to be ordered in the usual way: First
nodes in order of ID, then ways in order of ID, then relations in order of ID.
Negative IDs are allowed, they must be ordered before the positive IDs. See
the [**osmium-sort**(1)](osmium-sort.html) man page for details of the ordering.

This command will only work for OSM data files, not OSM history files or
change files.

This commands reads its input file only once, ie. it can read from STDIN.

# OPTIONS

-i, \--show-ids
:   Print all missing IDs to STDOUT. If you don't specify this option, only a
    summary is shown.

-r, \--check-relations
:   Also check referential integrity of relations. Without this option, only
    nodes in ways are checked.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@

# MEMORY USAGE

**osmium check-refs** will do the check in one pass through the input data. It
needs enough main memory to store all temporary data.

Largest memory need will be about 1 bit for each node ID, that's roughly 860 MB
these days (February 2020). With the **\--check-relations/-r** option memory
use will be a bit bigger.


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

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-sort**(1)](osmium-sort.html)
* [Osmium website](https://osmcode.org/osmium-tool/)


