
# NAME

osmium-apply-changes - apply OSM change file(s) to OSM data file


# SYNOPSIS

**osmium apply-changes** \[*OPTIONS*\] *OSM-DATA-FILE* *OSM-CHANGE-FILE*...
**osmium apply-changes** \[*OPTIONS*\] *OSM-HISTORY-FILE* *OSM-CHANGE-FILE*...


# DESCRIPTION

Merges the content of all OSM change files and applies those changes to the OSM
data or history file.

Objects in the data or historyy file must be sorted by type, ID, and version.
Objects in change files need not be sorted, so it doesn't matter in what order
the change files are given or in what order they contain the data.

Changes can be applied to normal OSM data files or OSM history files with this
command. File formats will be autodetected from the file name suffixes, see
the **--with-history** option if that doesn't work.


# OPTIONS

-H, --with-history
:   Update an OSM history file (instead of a normal OSM data file). Both
    input and output must be history files. This option is usually not
    necessary, because history files will be detected from their file name
    suffixes, but if this detection doesn't work, you can force this mode
    with this option.

-r, --remove-deleted
:   Deprecated. Remove deleted objects from the output. This is now the
    default if your input file is a normal OSM data file ('.osm').

-s, --simplify
:   Deprecated. Only write the last version of any object to the output.
    This is now the default if your input file is a normal OSM data file
    ('.osm').


@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium apply-changes** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium apply-changes** keeps the contents of all the change files in main
memory. This will take roughly 10 times as much memory as the files take on
disk in *.osm.bz2* format.


# EXAMPLES

Apply changes in `362.osc.gz` to planet file and write result to `new.osm.pbf`:

    osmium apply-changes --output=new.osm.pbf planet.osm.pbf 362.osc.gz


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5), **osmium-merge-changes**(1), **osmium-derive-changes**(1)
* [Osmium website](http://osmcode.org/osmium-tool/)

