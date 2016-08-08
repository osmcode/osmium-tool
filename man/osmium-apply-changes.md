
# NAME

osmium-apply-changes - apply OSM change file(s) to OSM data file


# SYNOPSIS

**osmium apply-changes** \[*OPTIONS*\] *OSM-DATA-FILE* *OSM-CHANGE-FILE*...


# DESCRIPTION

Merges the content of all OSM change files and applies those changes to the OSM
data file.

Objects in change files will be sorted by type, ID, and version, so it doesn't
matter in what order the change files are given or in what order they contain
the data.


# OPTIONS

-r, --remove-deleted
:   Remove deleted objects from the output. If this is not set, deleted objects
    will be in the output with the visible flag set to false.

-s, --simplify
:   Only write the last version of any object to the output.

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
* [Osmium website](http://osmcode.org/osmium)

