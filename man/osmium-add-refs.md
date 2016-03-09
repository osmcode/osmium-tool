
# NAME

osmium-add-refs - add referenced objects to OSM file


# SYNOPSIS

**osmium add-refs** \[*OPTIONS*\] -s OSM-SOURCE-FILE *OSM-FILE*...

**osmium add-refs** \[*OPTIONS*\] -s OSM-SOURCE-FILE -i *ID-FILE*


# DESCRIPTION

If one or more OSM files are given as input, start from the objects given in
those files. The order of the objects in the file does not matter.

If an ID file is given with the -i/--id-file option, read the IDs from this
file. IDs in this file have the form: *TYPE-LETTER* *NUMBER*. The type letter
is 'n' for nodes, 'w' for ways, and 'r' for relations. If there is no type
letter, 'n' for nodes is assumed. Each line of the file must start with an ID
in this format. Lines can optionally contain a space character after the ID.
Any characters after that are ignored. (This allows files in OPL format to be
read.) Empty lines and lines starting with '#' are also ignored. The order in
which the IDs appear does not matter.

Then find all missing references to way nodes and relation members, resolve
them recursively using the source file given with the -s/--source option and
write everything out.

Note that all objects will be taken from the source file, the other files are
only used to detect which objects to get. This might matter if there are
different object versions in the source files and in the other input files.

The input files will be read once. The source file will be read up to three
times to resolve all references.

Objects will be written out in the order they are found in the source file.

The source file can not be a history file unless the **-H**, **--history**
option is given.

If referenced objects are missing from the source file, the type and IDs
of those objects is written out to *stderr* at the end of the program unless
the **-H**, **--history** option was given.


# OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    **See osmium-file-formats**(5) or the libosmium manual for details.

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
    Make this program work on history files.

-i, --id-file=FILE
:   Read IDs from given file instead of from OSM files.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-s, --source=FILE
:   The missing references are taken from this file. The file will potentially
    be read several times.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium add-refs** exits with exit code

0
  ~ if everything went alright,

1
  ~ if some referred to objects could not be found in the source file (this is
    only detected when the **-h**, **--history** option was not used),

2
  ~ if there was a problem with the command line arguments.

# MEMORY USAGE

This command has to keep the list of all needed objects IDs in memory.


# EXAMPLES

Find all objects in `data.osm` or referenced by objects in `data.osm` in
`source.osm` and write them to the file `out.osm`:

    osmium add-refs -s source.osm -o out.osm data.osm


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

