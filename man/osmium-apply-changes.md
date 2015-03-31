% OSMIUM-APPLY-CHANGES(1)
% Jochen Topf <jochen@topf.org>

# NAME

osmium-apply-changes - apply OSM change file(s) to OSM data file


# SYNOPSIS

**osmium apply-changes** \[*OPTIONS*\] *INPUT-FILE* *CHANGE-FILE*...


# DESCRIPTION

Merges the content of all change files given on the command line and applies
those changes to the INPUT-FILE.

Objects in change files are sorted by type, ID, and version, so it doesn't matter
in what order the change files are given or in what order they contain the data.

**Osmium apply** keeps the contents of the change files in main memory, so the
data has to fit in there!


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

-r, --remove-deleted
:   Remove deleted objects from the output. If this is not set, deleted objects
    will be in the output with the visible flag set to false.

-s, --simplify
:   Only write the last version of any object to the output.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium cat** exits with code 0 if everything went alright, it exits
with code 2 if there was a problem with the command line arguments,
and with exit code 1 if some other error occurred.


# EXAMPLES

Apply change file 362.osc.gz to planet file:

    osmium apply-changes -o new.osm.pbf planet.osm.pbf 362.osc.gz


# SEE ALSO

* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

