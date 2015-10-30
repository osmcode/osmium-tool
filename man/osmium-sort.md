
# NAME

osmium-sort - sort OSM files


# SYNOPSIS

**osmium sort** \[*OPTIONS*\] *INPUT-FILE*...


# DESCRIPTION

Merges the content of all input files given on the command line and sort the
result. Objects are sorted by type, ID, and version.

This works with normal OSM data files, history files, and change files.

**osmium sort** does its work in main memory, so all data has to fit in there!


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

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.


# DIAGNOSTICS

**osmium sort** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# EXAMPLES

Sort *in.osm.bz2* and write out to *sorted.osm.pbf*:

    osmium sort -o sorted.osm.pbf in.osm.bz2


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

