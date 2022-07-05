
# NAME

osmium-cat - concatenate OSM files and convert to different formats


# SYNOPSIS

**osmium cat** \[*OPTIONS*\] *OSM-FILE*...


# DESCRIPTION

Concatenates all input files and writes the result to the output file. The data
is not sorted in any way but strictly copied from input to output.

Because this program supports several different input and output formats, it
can be used to convert OSM files from one format into another.

This commands reads its input file(s) only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.

Usually this is not the right command to merge two or more typical OSM files,
see **osmium merge** for that.

# OPTIONS

-c, \--clean=ATTR
:   Clean the attribute (*version*, *timestamp*, *changeset*, *uid*, *user*),
    from the data before writing it out again. The attribute will be set to 0
    (the user will be set to the empty string). This option can be given
    multiple times. Depending on the output format these attributes might
    show up as 0 or not show up at all.

-t, \--object-type=TYPE
:   Read only objects of given type (*node*, *way*, *relation*, *changeset*).
    By default all types are read. This option can be given multiple times.

\--buffer-data
:   Read all input files into memory and only then write out all the data.
    This will need a lot of memory and is usually slower than a normal copy.
    Used for timing the reading and writing phase separately.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium cat** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium cat** does all its work on the fly and doesn't keep much data in
main memory.


# EXAMPLES

Convert a PBF file to a compressed XML file:

    osmium cat -o out.osm.bz2 in.osm.pbf

Concatenate all change files in the 'changes' directory into one:

    osmium cat -o all-changes.osc.gz changes/*.osc.gz

Copy nodes and ways from source to destination file:

    osmium cat -o dest.osm.pbf source.osm.pbf -t node -t way

Remove changeset, uid, and user from a file to protect personal data:

    osmium cat -c changeset -c uid -c user -o cleaned.osm.pbf data.osm.pbf


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-merge**(1)](osmium-merge.html),
  [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

