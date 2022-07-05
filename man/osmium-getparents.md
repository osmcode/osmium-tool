
# NAME

osmium-getparents - get parents of objects from OSM file


# SYNOPSIS

**osmium getparents** \[*OPTIONS*\] *OSM-FILE* *ID*...\
**osmium getparents** \[*OPTIONS*\] *OSM-FILE* -i *ID-FILE*\
**osmium getparents** \[*OPTIONS*\] *OSM-FILE* -I *ID-OSM-FILE*


# DESCRIPTION

Get objects referencing the objects with the specified IDs from the input and
write them to the output. So this will get ways referencing any of the
specified node IDs and relations referencing any specified node, way, or
relation IDs. Only one level of indirection is resolved, so no relations of
relations are found and no relations referencing ways referencing the specified
node IDs.

IDs can be specified on the command line (first case in synopsis), or read from
text files with one ID per line (second case in synopsis), or read from OSM
files (third cases in synopsis). A mixture of these cases is also allowed.

All objects with these IDs will be read from *OSM-FILE* and written to the
output. If the option **\--add-self/-s** is specified, the objects with
the specified IDs themselves will also be added to the output.

Objects will be written out in the order they are found in the *OSM-FILE*.

The input file is read only once.

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
only used to detect which objects to get. This might matter if there are
different object versions in the different files.

The *OSM-FILE* can be a history file, then all matching versions of the objects
will be copied to the output.

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
:   Like **-i** but get the IDs from an OSM file. This option can be used
    multiple times.

-s, \--add-self
:   Also add all objects with the specified IDs to the output.

\--verbose-ids
:   Also print all requested IDs. This is usually disabled, because
    the lists can get quite long. (This option implies **\--verbose**.)

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium getparents** exits with exit code

0
  ~ if there was no error.

1
  ~ if there was an error processing the data.

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium getparents** does all its work on the fly and only keeps a table of
all IDs it needs in main memory.


# EXAMPLES

Output all ways referencing nodes 17 or 1234, and all relations with nodes 17
or 1234, or way 42, or relation 111 as members to STDOUT in OPL format:

    osmium getparents -f opl planet.osm.pbf n1234 w42 n17 r111


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-getid**(1)](osmium-getid.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

