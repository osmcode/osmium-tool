
# NAME

osmium-getid - get objects from OSM file by ID


# SYNOPSIS

**osmium getid** \[*OPTIONS*\] *OSM-FILE* *ID*...


# DESCRIPTION

Get objects with the given IDs from the input and write them to the output.

IDs have the form: *TYPE-LETTER* *NUMBER*. The type letter is 'n' for nodes,
'w' for ways, and 'r' for relations. If there is no type letter, 'n' for nodes
is assumed. So "n13 w22 17 r21" will match the nodes 13 and 17, the way 22 and
the relation 21. The order in which the IDs appear does not matter.

The list of IDs can be in separate arguments or in a single argument separated
by spaces, tabs, commas (,), semicolons (;), forward slashes (/) or pipe
characters (|).


# OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    See **osmium-file-formats**(5) or the libosmium manual for details.

-F, --input-format=FORMAT
:   The format of the input files. Can be used to set the input format if it
    can't be autodetected from the file names. This will set the format for
    all input files, there is no way to set the format for some input files
    only. See **osmium-file-formats**(5) or the libosmium manual for details.

--fsync
:   Call fsync after writing the output file to force the OS to flush buffers
    to disk.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

-h, --help
:   Show usage help.

-i, --id-file=FILE
:   Read IDs from file instead of from the command line. Each line of the
    file must start with an ID in the format described above. Lines can
    optionally contain a space character after the ID. Any characters after
    that are ignored. (This allows files in OPL format to be read.) Empty
    lines and lines starting with '#' are also ignored.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium getid** exits with exit code

0
  ~ if all IDs were found

1
  ~ if there was an error processing the data or not all IDs were found, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium getid** does all its work on the fly and doesn't keep much data in
main memory.


# EXAMPLES

Output nodes 17 and 1234, way 42, and relation 111 to *stdout* in OPL format:

    osmium getid -f opl planet.osm.pbf n1234 w42 n17 r111


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

