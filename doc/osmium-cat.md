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

--generator
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is *osmium/* and the version
    of osmium.

--input-format, -F
:   The format of the input files. Can be used to set the input format if it
    can't be autodetected from the file names. This will set the format for
    all input files, there is no way to set the format for some input files
    only. See the FILE FORMATS section for details.

--output, -o
:   Name of the output file. Default is '-' (*stdout*).

--output-format, -f
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name. See the FILE FORMATS
    section for details.

--output-header
:   Add output header. This option can be given several times.

--overwrite, -O
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--verbose,-v
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# FILE FORMATS

**osmium** supports all file formats supported by Osmium. These are:

* The classical XML format in the variants *.osm* (for data files),
  *.osh* (for data files with history) and *.osc* (for change files)
* The PBF binary format (with the usual suffix *.osm.pbf*)
* The OPL format (with the usual suffix *.osm.opl*)

In addition XML and OPL files can be compressed using *gzip* or *bzip2*.
(Add *.gz* or *.bz2* suffixes, respectively.)

Which format a file has is usually autodetected from the file name.
Where this doesn't work, either because you are reading from stdin
or writing to stdout, or because you have an unusual file name, the
--input-format and --output-format options can be used to set the
format. They take a comma-separated list of arguments, the first is
the format, further arguments set additional options.

Here are some examples:

pbf
:   PBF format
osm.bz2
:   XML format, compressed with bzip2
osc.gz
:   OSM change file, compressed with gzip
osm.gz,xml_change_format=true
:   OSM change file, compressed with gzip


# DIAGNOSTICS

**osmium cat** exits with code 0 if everything went alright, it exits
with code 2 if there was a problem with the command line arguments,
and with exit code 1 if some other error occurred.


# EXAMPLES

Convert a PBF file to a compressed XML file:

    osmium cat -o out.osm.bz2 in.osm.pbf

Concatenate all change files in the 'changes' directory into one:

    osmium cat -o all-changes.osc.gz changes/*.osc.gz


# COPYRIGHT

Copyright (C) 2013  Jochen Topf <jochen@topf.org>.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.


# SEE ALSO

<http://osmcode.org/osmium>

