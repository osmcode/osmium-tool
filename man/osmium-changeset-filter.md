
# NAME

osmium-changeset-filter - filter changesets from OSM changeset file


# SYNOPSIS

**osmium changeset-filter** \[*OPTIONS*\] *OSM-CHANGESET-FILE*


# DESCRIPTION

Copy the changesets matching all the given criteria to the output. Matching
criteria are given through command line options.

This commands reads its input file only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.

# FILTER OPTIONS

-a, \--after=TIMESTAMP
:   Only copy changesets closed after the given time.
    This will always include all open changesets.

-b, \--before=TIMESTAMP
:   Only copy changesets created before the given time.

-B, \--bbox=LONG1,LAT1,LONG2,LAT2
:   Only copy changesets with a bounding box overlapping the specified box.
    The coordinates LONG1,LAT1 are from one arbitrary corner, the coordinates
    LONG2,LAT2 are from the opposite corner.

-c, \--with-changes
:   Only copy changesets with changes.

-C, \--without-changes
:   Only copy changesets without changes.

-d, \--with-discussion
:   Only copy changesets with discussions, ie changesets with at least one
    comment.

-D, \--without-discussion
:   Only copy changesets without discussions, ie changesets without any
    comments.

\--open
:   Only copy open changesets.

\--closed
:   Only copy closed changesets.

-u, \--user=USER
:   Only copy changesets by the given user name.

-U, \--uid=UID
:   Only copy changesets by the given user ID.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium changeset-filter** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium changeset-filter** does all its work on the fly and doesn't keep much
data in main memory.


# EXAMPLES

To see all changesets by user "foo":

    osmium changeset-filter -u foo -f debug changesets.osm.bz2

To create an OPL file containing only open changesets:

    osmium changeset-filter --open -o open-changesets.opl.bz2 changesets.osm.bz2


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

