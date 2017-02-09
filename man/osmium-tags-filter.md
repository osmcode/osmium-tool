
# NAME

osmium-tags-filter - get objects with keys/tags from OSM file


# SYNOPSIS

**osmium tags-filter** \[*OPTIONS*\] *OSM-FILE* *FILTER-EXPRESSION*...\
**osmium tags-filter** \[*OPTIONS*\] --expressions=*FILE* *OSM-FILE*


# DESCRIPTION

Get objects matching the specified expressions from the input and write them to
the output. Expressions can either be specified on the command line or in an
expressions file, one per line. See the **FILTER EXPRESSIONS** section for a
description of the filter expression format.

All objects matching the expressions will be read from *OSM-FILE* and written
to the output. If the option **-r**, **--add-referenced** is used all objects
referenced from those objects will also be added to the output.

If the option **-r**, **--add-referenced** is *not* used, the input file is
read only once, if it is used, the input file will possibly be read up to
three times.

Objects will be written out in the order they are found in the *OSM-FILE*.

The *OSM-FILE* will work correctly on history files unless the
**-r**/**--add-referenced** option is used.


# OPTIONS

-e FILE, --expressions=FILE
:   Read expressions from the specified file, one per line. Empty lines are
    ignored. Everything after the comment character (#) is also ignored. The
    the **FILTER EXPRESSIONS** section for further details.

-r, --add-referenced
:   Recursively find all objects referenced by the matching objects and include
    them in the output. This only works correctly on non-history files.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@


# FILTER EXPRESSIONS

A filter expression specifies a tag or tags that should be found in the data
and the type of object (node, way, or relation) that should be matched.

The object type(s) comes first, then a slash (/) and then the rest of the
expression. Object types are specified as 'n' (for nodes), 'w' (for ways), and
'r' (for relations). Any combination of them can be used. If the object type is
not specified, the expression matches all object types.

Some examples:

n/amenity
:   Matches all nodes with the key "amenity".

nw/highway
:   Matches all nodes or ways with the key "highway".

/note
:   Matches objects of any type with the key "note".

note
:   Matches objects of any type with the key "note".

The filter expressions specified in a file and/or on the command line are
matched in the order they are given. To achieve best performance, put
expressions expected to match more often first.


# DIAGNOSTICS

**osmium tags-filter** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium tags-filter** does all its work on the fly and only keeps tables of
object IDs it needs in main memory if the **-r**/**--add-references** option
is used.


# EXAMPLES

Get all amenity nodes from the Berlin PBF file:

    osmium tags-filter -o amenties.osm.pbf berlin.osm.pbf n/amenity

Get all objects (nodes, ways, or relations) with a `note` tag:

    osmium tags-filter -o notes.osm.pbf berlin.osm.pbf note

Get all nodes and ways with a `highway` tag and all relations tagged with
`type=restriction` plus all referenced objects:

    osmium tags-filter -r -o filtered.osm.pbf planet.osm.pbf \
        nw/highway r/type=restriction


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium-tool/)

