
# NAME

osmium-tags-filter - filter objects matching specified keys/tags


# SYNOPSIS

**osmium tags-filter** \[*OPTIONS*\] *OSM-FILE* *FILTER-EXPRESSION*...\
**osmium tags-filter** \[*OPTIONS*\] \--expressions=*FILE* *OSM-FILE*


# DESCRIPTION

Get objects matching at least one of the specified expressions from the input
and write them to the output. Expressions can either be specified on the
command line or in an expressions file. See the **FILTER EXPRESSIONS** section
for a description of the filter expression format.

All objects matching the expressions will be read from *OSM-FILE* and written
to the output. All objects referenced from those objects will also be added
to the output unless the option **\--omit-referenced/-R** is used. This
applies to nodes referenced in ways and members referenced in relations.

If the option **\--omit-referenced/-R** is used, the input file is read
only once, otherwise the input file will possibly be read up to three times.

Objects will be written out in the order they are found in the *OSM-FILE*.

The command will only work correctly on history files if the
**\--omit-referenced/-R** option is used. The command can not be used on
change files.


# OPTIONS

-e FILE, \--expressions=FILE
:   Read expressions from the specified file, one per line. Empty lines are
    ignored. Everything after the comment character (#) is also ignored. See
    the **FILTER EXPRESSIONS** section for further details.

-i, \--invert-match
:   Invert the sense of matching. Exclude all objects with matching tags.

-R, \--omit-referenced
:   Omit the nodes referenced from matching ways and members referenced from
    matching relations.

-t, \--remove-tags
:   Remove tags from objects that are not matching the filter expression but
    are included to complete references (nodes in ways and members of
    relations). If an object is both matching the filter and used as a
    reference it will keep its tags.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@


# FILTER EXPRESSIONS

A filter expression specifies a tag or tags that should be found in the data
and the type of object (node, way, or relation) that should be matched.

The object type(s) comes first, then a slash (/) and then the rest of the
expression. Object types are specified as 'n' (for nodes), 'w' (for ways),
'r' (for relations), and 'a' (for areas - closed ways with 4 or more nodes and
relations with `type=multipolygon` or `type=boundary` tag). Any combination of
them can be used. If the object type is not specified, the expression matches
all object types.

Some examples:

n/amenity
:   Matches all nodes with the key "amenity".

nw/highway
:   Matches all nodes or ways with the key "highway".

/note
:   Matches objects of any type with the key "note".

note
:   Matches objects of any type with the key "note".

w/highway=primary
:   Matches all ways with the key "highway" and value "primary".

w/highway!=primary
:   Matches all ways with the key "highway" and a value other than "primary".

r/type=multipolygon,boundary
:   Matches all relations with key "type" and value "multipolygon" or "boundary".

w/name,name:de=Kastanienallee,Kastanienstrasse
:   Matches any way with a "name" or "name:de" tag with the value
    "Kastanienallee" or "Kastanienstrasse".

n/addr:\*
:   Matches all nodes with any key starting with "addr:"

n/name=\*Paris
:   Matches all nodes with a name that contains the word "Paris".

a/building
:   Matches any closed ways with 4 or more nodes or relations tagged
    "building". Relations must also have a tag "type=multipolygon" or
    "type=boundary".

If there is no equal sign ("=") in the expression only keys are matched and
values can be anything. If there is an equal sign ("=") in the expression, the
key is to the left and the value to the right. An exclamation sign ("!") before
the equal sign means: A tag with that key, but not the value(s) to the right of
the equal sign. A leading or trailing asterisk ("\*") can be used for substring
or prefix matching, respectively. Commas (",") can be used to separate several
keys or values.

All filter expressions are case-sensitive. There is no way to escape the
special characters such as "=", "\*" and ",". You can not mix
comma-expressions and "\*"-expressions.

The filter expressions specified in a file and/or on the command line are
matched in the order they are given. To achieve best performance, put
expressions expected to match more often first.

Area matches (with leading "a/") do not check whether the matched object is a
valid (multi)polygon, they only check whether an object might possibly be
turned into a (multi)polygon. This is the case for all closed ways (where the
first and last node are the same) with 4 or more nodes and for all relations
that have an additional "type=multipolygon" or "type=boundary" tag.


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
object IDs it needs in main memory. If the **\--omit-referenced/-R** option
is used, no IDs are kept in memory.


# EXAMPLES

Get all amenity nodes from the Berlin PBF file:

    osmium tags-filter -o amenties.osm.pbf berlin.osm.pbf n/amenity

Get all objects (nodes, ways, or relations) with a `note` tag:

    osmium tags-filter -R -o notes.osm.pbf berlin.osm.pbf note

Get all nodes and ways with a `highway` tag and all relations tagged with
`type=restriction` plus all referenced objects:

    osmium tags-filter -o filtered.osm.pbf planet.osm.pbf \
        nw/highway r/type=restriction


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

