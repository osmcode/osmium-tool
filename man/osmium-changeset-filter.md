
# NAME

osmium-changeset-filter - filter changesets from OSM changeset file


# SYNOPSIS

**osmium changeset-filter** \[*OPTIONS*\] *INPUT-FILE*


# DESCRIPTION

Copy the changesets matching all the given criteria to the output. Matching
criteria are given through command line options.


# OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    **See osmium-file-formats**(5) or the libosmium manual for details.

-F, --input-format=FORMAT
:   The format of the input file. Can be used to set the input format if it
    can't be autodetected from the file names. See **osmium-file-formats**(5)
    or the libosmium manual for details.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.

-d, --with-discussion
:   Only copy changesets with discussions, ie changesets with at least one
    comment.

-D, --without-discussion
:   Only copy changesets without discussions, ie changesets without any
    comments.

-c, --with-changes
:   Only copy changesets with changes.

-C, --without-changes
:   Only copy changesets without changes.

--open
:   Only copy open changesets.

--closed
:   Only copy closed changesets.

-u, --user=USER
:   Only copy changesets by the given user name.

-U, --uid=UID
:   Only copy changesets by the given user ID.

-a, --after=TIMESTAMP
:   Only copy changesets closed after the given time.
    This will always include all open changesets.

-b, --before=TIMESTAMP
:   Only copy changesets created before the given time.


# DIAGNOSTICS

**osmium changeset-filter** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# EXAMPLES

To see all changesets by user "foo":

    osmium changeset-filter -u foo -f debug changesets.osm.bz2

To create an OPL file containing only open changesets:

    osmium changeset-filter --open -o open-changesets.opl.bz2 changesets.osm.bz2


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

