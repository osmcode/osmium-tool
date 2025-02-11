
# NAME

osmium-time-filter - filter OSM data by time from a history file


# SYNOPSIS

**osmium time-filter** \[*OPTIONS*\] *OSM-HISTORY-FILE* \[*TIME*\]\
**osmium time-filter** \[*OPTIONS*\] *OSM-HISTORY-FILE* *FROM-TIME* *TO-TIME*


# DESCRIPTION

Copy all objects that were valid at the given *TIME* or in the time period
between *FROM-TIME* (inclusive) and *TO-TIME* (not inclusive) from the input
file into the output file.  If no time is given, the current time is used.

Usually the *INPUT-FILE* will be an OSM data file with history. If both
*FROM-TIME* and *TO-TIME* are given, the result will also have history data,
it will also include deleted versions of objects.

If only a single point in time was given, the result will be a normal OSM file
without history containing no deleted objects.

The format for the timestamps is "yyyy-mm-ddThh:mm:ssZ".

This commands reads its input file only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.

If the input file has a timestamp set in the header and the specified *TO-TIME*
is before that timestamp in the header, the header of the output file will have
the timestamp set to the *TO-TIME* (or *TO-TIME* minus one second if a
*FROM-TIME* was also set).

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium time-filter** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium time-filter** does all its work on the fly and doesn't keep much data
in main memory.


# EXAMPLES

Extract current planet file from history planet:

    osmium time-filter -o planet.osm.pbf history-planet.osh.pbf

Extract planet data how it appeared on January 1 2008 from history planet:

    osmium time-filter -o planet-20080101.osm.pbf history-planet.osh.pbf 2008-01-01T00:00:00Z


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

