
# NAME

osmium-diff - display differences between OSM files


# SYNOPSIS

**osmium diff** \[*OPTIONS*\] *OSM-FILE1* *OSM-FILE2*


# DESCRIPTION

Finds all differences between two OSM files and displays them. This command
compares all attributes of all objects, so it will even find, say, differences
in the user name or even the order of tags, differences that should not happen
in normal OSM data unless there is also a different object version.

Only differences between objects (node, ways, and relations) are found and
displayed. Headers are ignored.

Objects in both input files must be sorted in the same order.

Several output formats are supported, see the **OUTPUT FORMATS** section.

This command is intended for displaying the differences between files to
humans. It can not be used to create an OSM change file (`.osc`), use
**osmium-derive-changes** for that.


# OUTPUT FORMATS

The following output formats are supported and can be set with the
**\--output-format/-f** options. Default is the compact format.

compact
:   A very compact format. For all objects a line is printed with the type
    of object ('n', 'w', or 'r'), the object ID and then the version number.
    If objects appear in both files and are identical they are preceded by
    a space (' ') character, if they are in both files, but different, they
    are preceded by an asterisk ('*'). Otherwise they get a minus ('-') or
    plus ('+') character to show that they are only in the first or second
    file, respectively.

opl
:   The usual OPL format with all lines preceded by space (' '), minus
    ('-'), or plus ('+') characters depending on whether the object is in both,
    the first, or the second file.

debug
:   The usual debug format with all lines preceded by space (' '), minus
    ('-'), or plus ('+') characters depending on whether the object is in both,
    the first, or the second file. Color support can be enabled ('debug,color').

None of the output formats print the headers of the input files.


# OPTIONS

-c, \--suppress-common
:   Do not output objects that are the same in both files.

-f, \--output-format=FORMAT
:   See the **OUTPUT FORMATS** section.

\--ignore-changeset
:   Ignore changeset id on OSM objects when comparing files. When used with
    the OPL output format the 'changeset' attribute is not written to the
    output.

\--ignore-uid
:   Ignore user id on OSM objects when comparing files. When used with
    the OPL output format the 'uid' attribute is not written to the output.

\--ignore-user
:   Ignore user name on OSM objects when comparing files. When used with
    the OPL output format the 'user' attribute is not written to the output.

-o, \--output=FILE
:   Name of the output file. Default is '-' (STDOUT).

-O, \--overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

-q, \--quiet
:   No output. Just report when files differ through the return code.

-s, \--summary
:   Print count of objects that are only in the left or right files, or the
    same in both or different in both to STDERR.

-t, \--object-type=TYPE
:   Read only objects of given type (*node*, *way*, *relation*).
    By default all types are read. This option can be given multiple times.
    This affects the output as well as the return code of the command.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@

# DIAGNOSTICS

**osmium diff** exits with exit code

0
  ~ if the files are the same,

1
  ~ if the files are different, or

2
  ~ if there was an error


# MEMORY USAGE

**osmium diff** doesn't keep a lot of data in memory.


# EXAMPLES

Show difference between Nepal files from January 2016 and February 2016 in
compact format:

    osmium diff nepal-20160101.osm.pbf nepal-20160201.osm.pbf

Show in color debug format only those objects that are different:

    osmium diff nepal-20160101.osm.pbf nepal-20160201.osm.pbf -f debug,color -c


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-derive-changes**(1)](osmium-derive-changes.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

