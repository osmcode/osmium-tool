
# NAME

osmium-removeid - remove objects from OSM file by ID


# SYNOPSIS

**osmium removeid** \[*OPTIONS*\] *OSM-FILE* *ID*...\
**osmium removeid** \[*OPTIONS*\] *OSM-FILE* -i *ID-FILE*\
**osmium removeid** \[*OPTIONS*\] *OSM-FILE* -I *ID-OSM-FILE*


# DESCRIPTION

Copy input file to output removing objects with the specified IDs.

IDs can be given on the command line (first case in synopsis), or read from
text files with one ID per line (second case in synopsis), or read from
OSM files (third cases in synopsis). A mixture of these cases is also allowed.

Objects will be written out in the order they are found in the *OSM-FILE*.
The input file is only read once, reading from *STDIN* is possible by using
the special file name '-'.

On the command line or in the ID file, the IDs have the form: *TYPE-LETTER*
*NUMBER*. The type letter is 'n' for nodes, 'w' for ways, and 'r' for
relations. If there is no type letter, 'n' for nodes is assumed (or whatever
the **\--default-type** option says). So "n13 w22 17 r21" will match the nodes
13 and 17, the way 22 and the relation 21.

The order in which the IDs appear does not matter. Identical IDs can appear
multiple times on the command line or in the ID file(s).

On the command line, the list of IDs can be in separate arguments or in a
single argument separated by spaces, tabs, commas (,), semicolons (;), forward
slashes (/) or pipe characters (|).

In an ID file (option **\--id-file/-i**) each line must start with an ID in
the format described above. Leading space characters in the line are ignored.
Lines can optionally contain a space character or a hash sign ('#') after the
ID. Any characters after that are ignored. (This also allows files in OPL
format to be read.) Empty lines are ignored.

Note that all objects will be taken from the *OSM-FILE*, the *ID-OSM-FILE* is
only used to detect which objects to remove.

The *OSM-FILE* can be a history file in which case all versions of the objects
with the specified IDs will be removed.

This command will not work with negative IDs.


# OPTIONS

\--default-type=TYPE
:   Use TYPE ('node', 'way', or 'relation') for IDs without a type prefix
    (default: 'node'). It is also allowed to just use the first character
    of the type here.

-i, \--id-file[=FILE]
:   Read IDs from text file instead of from the command line. Use the special
    name "-" to read from *STDIN*. Each line of the file must start with an
    ID in the format described above. Lines can optionally contain a space
    character or a hash sign ('#') after the ID. This character and all
    following characters are ignored. (This allows files in OPL format to be
    read.) Empty lines are also ignored. This option can be used multiple
    times.

-I, \--id-osm-file=OSMFILE
:   Like **\--id-file/-i** but get the IDs from an OSM file. This option can be
    used multiple times.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium removeid** exits with exit code

0
  ~ if nothing went wrong

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium removeid** does all its work on the fly and only keeps a table of all
IDs it needs in main memory.


# EXAMPLES

Output all nodes except nodes 17 and 1234, all ways except way 42, and all
relations except relation 111 to STDOUT in OPL format:

    osmium removeid -f opl planet.osm.pbf n1234 w42 n17 r111


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-getid**(1)](osmium-getid.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

