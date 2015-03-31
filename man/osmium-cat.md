% OSMIUM-CAT(1)
% Jochen Topf <jochen@topf.org>

# NAME

osmium-cat - Concatenate OSM files and convert to different formats.


# SYNOPSIS

**osmium cat** \[OPTIONS\] \[**-o** *OUTPUT-FILE*\] *INPUT-FILE...*


# DESCRIPTION

Concatenates all input files and writes the result to the output file. The data
is not sorted in any way but strictly copied from input to output.

Because this program supports several different input and output formats, it
can be used to convert OSM files from one format into another.


# OPTIONS

--generator NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

--input-format, -F FORMAT
:   The format of the input files. Can be used to set the input format if it
    can't be autodetected from the file names. This will set the format for
    all input files, there is no way to set the format for some input files
    only. See **osmium-file-formats**(5) or the libosmium manual for details.

--output, -o FILE
:   Name of the output file. Default is '-' (*stdout*).

--output-format, -f FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    **See osmium-file-formats**(5) or the libosmium manual for details.

--output-header OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

--overwrite, -O
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--object-type, -t TYPE
:   Read only objects of given type (*node*, *way*, *relation*, *changeset*).
    By default all types are read. This option can be given multiple times.

--verbose, -v
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium cat** exits with code 0 if everything went alright, it exits
with code 2 if there was a problem with the command line arguments,
and with exit code 1 if some other error occurred.


# EXAMPLES

Convert a PBF file to a compressed XML file:

    osmium cat -o out.osm.bz2 in.osm.pbf

Concatenate all change files in the 'changes' directory into one:

    osmium cat -o all-changes.osc.gz changes/*.osc.gz

Copy nodes and ways from source to destination file:

    osmium cat -o dest.osm.pbf source.osm.pbf -t node -t way


# SEE ALSO

* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

