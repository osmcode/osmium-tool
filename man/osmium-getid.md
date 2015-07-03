
# NAME

osmium-getid - get objects from OSM file with given IDs


# SYNOPSIS

**osmium getid** \[*OPTIONS*\] *INPUT-FILE* *ID*...


# DESCRIPTION

Get objects with the given IDs from the input and write them to the output


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

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium getid** exits with code 0 if everything went alright and all IDs
were found, it exits with code 2 if there was a problem with the command
line arguments, and with exit code 1 if not all IDs were found.


# EXAMPLES

Output nodes 17 and 1234, way 42, and relation 111 to *stdout* in OPL format:

    osmium getid -f opl planet.osm.pbf n1234 w42 n17 r111


# SEE ALSO

* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

