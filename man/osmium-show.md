
# NAME

osmium-show - show OSM file


# SYNOPSIS

**osmium show** \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Show the contents of the *OSM-FILE* on STDOUT, usually in a pager. The
output format can be set using the **output-format/-f** option, its
shortcuts **-d** (debug format with colors), **-o** (OPL), or **-x** (XML),
or the `OSMIUM_SHOW_FORMAT` environment variable.

The pager can be set with the `OSMIUM_PAGER` or the `PAGER` environment
variable. If neither is set, the default `less` is used unless the option
**\--no-pager** is used. If the pager variables are set to an empty value or
to `cat`, no pager is used. On Windows there is no pager support at all.

This commands reads its input file only once, ie. it can read from STDIN.


# OPTIONS

-f, \--output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    **See osmium-file-formats**(5) or the libosmium manual for details.

\--no-pager
:   Disable pager.

-d, \--format-debug
:   Same as `-f debug,color=true`.

-o, \--format-opl
:   Same as `-f opl`.

-x, \--format-xml
:   Same as `-f xml`.

-t, \--object-type=TYPE
:   Read only objects of given type (*node*, *way*, *relation*, *changeset*).
    By default all types are read. This option can be given multiple times.


# COMMON OPTIONS

-h, \--help
:   Show usage help.

@MAN_INPUT_OPTIONS@

# DIAGNOSTICS

**osmium show** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium show** does all its work on the fly and doesn't keep much data in
main memory.


# EXAMPLES

Show an OSM file using the default pager and default format:

    osmium show norway.osm.pbf

Use `more` as a pager and only show relations:

    OSMIUM_PAGER=more osmium show -t r norway.osm.pbf

Show using XML format:

    osmium show -x norway.osm.pbf


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-cat**(1)](osmium-cat.html), [**osmium-file-formats**(5)](osmium-file-formats.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

