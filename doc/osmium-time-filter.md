% OSMIUM-TIME-FILTER(1)
% Jochen Topf <jochen@topf.org>

# NAME

osmium-time-filter - Filter OSM data from a point in time or a time span out of a history file.


# SYNOPSIS

**osmium time-filter** \[OPTIONS\] \[**-o** *OUTPUT-FILE*\] *INPUT-FILE* \[*TIME*\]\
**osmium time-filter** \[OPTIONS\] \[**-o** *OUTPUT-FILE*\] *INPUT-FILE* *FROM-TIME* *TO-TIME*


# DESCRIPTION

Copy all objects that were valid at the given *TIME* or in the time period
between *FROM-TIME* (inclusive) and *TO-TIME* (not inclusive) from the input
file into the output file.  If no time is given on the command line the current
time is used.

Usually the *INPUT-FILE* will be an OSM data file with history. If both *FROM-TIME*
and *TO-TIME* are given, the result will also have history data, it will also
include deleted versions of objects.

If only a single point in time was given the result will be a normal OSM file
without history containing no deleted objects.

The format for the time stamps is "yyyy-mm-ddThh:mm::ssZ".


# OPTIONS

--generator
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is *osmium/* and the version
    of osmium.

--input-format, -F
:   The format of the input file. Can be used to set the input format if it
    can't be autodetected from the file name. See the FILE FORMATS section
    of the osmium-cat man page for details.

--output-file, -o
:   Name of the output file. Default is '-' (*stdout*).

--output-format, -f
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name. See the FILE FORMATS
    section of the osmium-cat man page for details.

--overwrite, -O
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--verbose,-v
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium time-filter** exits with code 0 if everything went alright, it exits
with code 2 if there was a problem with the command line arguments,
and with exit code 1 if some other error occurred.


# EXAMPLES

Extract current planet file from history planet:

    osmium time-filter -o planet.osm.pbf history-planet.osh.pbf

Extract planet data how it appeared on January 1 2008 from history planet:

    osmium time-filter -o planet-20080101.osm.pbf history-planet.osh.pbf 2008-01-01T00:00:00Z


# COPYRIGHT

Copyright (C) 2013  Jochen Topf <jochen@topf.org>.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.


# SEE ALSO

<http://osmcode.org/osmium>

