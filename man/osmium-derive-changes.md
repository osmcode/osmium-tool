
# NAME

osmium-derive-changes - create OSM change files from two OSM data files


# SYNOPSIS

**osmium derive-changes** \[*OPTIONS*\] *OSM-FILE1* *OSM-FILE2*


# DESCRIPTION

Finds differences between two OSM files and creates a change file with those
differences. The resulting change file is created in a way, that it could be
applied on *OSM-FILE1* to re-create *OSM-FILE2*.

Objects in both input files must be sorted by type, ID, and version. The first
input file must be from a point in time before the second input file.

Only object type, id, and version are compared, so this program will not detect
differences in, say, the tags, unless the object has a new version, which is
the normal way things work in OSM. If you need to compare all data in OSM
files, have a look at the **osmium diff** program.

For this command to create a proper change file you have to set the
**\--output** option or **\--output-format** option in a way that it will
generate an .osc file, typically by using something like '-o out.osc.gz'.
You can create any other OSM file format, but that is usually not what you
want. Osmium derive-changes will warn you in this case.

Note that for objects that are in *OSM-FILE1* but not *OSM-FILE2* a "deleted"
entry will be created in the output. But because we can not know when this
deletion actually occurred, in which changeset, and by which user, it is
unclear what attributes this deleted object should have. Also, if you are
working with extracts, the object might not actually have been deleted in the
OSM database, but just moved out of the extract you are looking at. So a real,
new, "deleted" version was never created. Usually the "deleted" object will get
the same version number and timestamp as the object in *OSM-FILE1* had, all
other information will be removed. But you can change this using the
**\--increment-version**, **\--keep-details**, and **\--update-timestamp**
options. Depending on which software you are using the change files with,
different settings might be necessary.

This commands reads its input files only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.


# OPTIONS

\--increment-version
:   Increment version number of deleted objects.

\--keep-details
:   Keep details of deleted objects. Usually only id, version, and timestamp
    are kept. If this option is set all attributes, all tags, and all nodes
    or members for ways and relations, respectively, are kept.

\--update-timestamp
:   Update timestamp of deleted objects to the current time. This is the same
    behaviour as Osmosis.


@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium derive-changes** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium derive-changes** doesn't keep a lot of data in memory.


# EXAMPLES

Find changes in Nepal extract in January 2016:

    osmium derive-changes nepal-20160101.osm.pbf nepal-20160201.osm.pbf -o nepal-jan.osc.bz2


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html),
  [**osmium-apply-changes**(1)](osmium-apply-changes.html), [**osmium-diff**(1)](osmium-diff.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

