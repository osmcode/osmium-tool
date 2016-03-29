
# NAME

osmium-getid - get objects from OSM file by ID


# SYNOPSIS

**osmium getid** \[*OPTIONS*\] *OSM-FILE* *ID*...
**osmium getid** \[*OPTIONS*\] *OSM-FILE* -i *ID-FILE*
**osmium getid** \[*OPTIONS*\] *OSM-FILE* -I *ID-OSM-FILE*


# DESCRIPTION

Get objects with the given IDs from the input and write them to the output.

IDs can be
* given on the command line (first case in synopsis), or
* read from a text file with one ID per line (second case in synopsis), or
* read from an OSM file (third cases in synopsis).

All objects with these IDs will be read from *OSM-FILE* and written to the
output. If the option **-r**, **--add-referenced** is used all objects
referenced from those objects will also be added to the output.

Objects will be written out in the order they are found in the *OSM-FILE*.

If the option **-r**, **--add-referenced** is *not* used, the input file is
read only once, if it is used, the input file will possibly be read up to
three times.

On the command line or in the ID file, the IDs have the form: *TYPE-LETTER*
*NUMBER*. The type letter is 'n' for nodes, 'w' for ways, and 'r' for
relations. If there is no type letter, 'n' for nodes is assumed (or whatever
the **--default-type** option says). So "n13 w22 17 r21" will match the nodes
13 and 17, the way 22 and the relation 21.

The order in which the IDs appear does not matter.

On the command line, the list of IDs can be in separate arguments or in a
single argument separated by spaces, tabs, commas (,), semicolons (;), forward
slashes (/) or pipe characters (|).

In an ID file (option **-i**/**--id-file**) each line must start with an ID in
the format described above. Lines can optionally contain a space character or a
hash sign ('#') after the ID. Any characters after that are ignored. (This also
allows files in OPL format to be read.) Empty lines are ignored.

Note that all objects will be taken from the *OSM-FILE*, the *ID-OSM-FILE* is
only used to detect which objects to get. This might matter if there are
different object versions in the different files.

The *OSM-FILE* can not be a history file unless the **-H**, **--history**
option is used. Then all versions of the objects will be copied to the ouput.

If referenced objects are missing from the input file, the type and IDs
of those objects is written out to *stderr* at the end of the program unless
the **-H**, **--history** option was given.


# OPTIONS

--default-type=TYPE
:   Use TYPE ('node', 'way', or 'relation') for IDs without a type prefix
    (default: 'node'). It is also allowed to just use the first character
    of the type here.

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

-H, --history
    Make this program work on history files. This is only needed when using
    the **-r** option.

-i, --id-file[=FILE]
:   Read IDs from text file instead of from the command line. Use without a
    filename or the special name "-" to read from *stdin*. Each line of the
    file must start with an ID in the format described above. Lines can
    optionally contain a space character or a hash sign ('#') after the ID.
    This character and all following characters are ignored. (This allows
    files in OPL format to be read.) Empty lines are also ignored.

-I, --id-osm-file=OSMFILE
:   Like **-i** but get the IDs from an OSM file.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-r, --add-referenced
:   Recursively find all objects referenced by the objects of the given IDs
    and include them in the output. This only works correctly on non-history
    files unless the `-H` option is also used.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.

--verbose-ids
:   Also print all requested and missing IDs. This is usually disabled, because
    the lists can get quite long. (This option implies `--verbose`.)


# DIAGNOSTICS

**osmium getid** exits with exit code

0
  ~ if all IDs were found

1
  ~ if there was an error processing the data or not all IDs were found,
    (this is only detected if the **-h**, **--history** option was not used),

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium getid** does all its work on the fly and only keeps a table of all
IDs it needs in main memory.


# EXAMPLES

Output nodes 17 and 1234, way 42, and relation 111 to *stdout* in OPL format:

    osmium getid -f opl planet.osm.pbf n1234 w42 n17 r111


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

