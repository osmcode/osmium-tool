
# NAME

osmium-export - export OSM data


# SYNOPSIS

**osmium export** \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

The OSM data model with its nodes, ways, and relations is very different from
the data model usually used for geodata with features having point, linestring,
or polygon geometries (or their cousins, the multipoint, multilinestring, or
multipolygon geometries).

The **export** command transforms OSM data into a more usual GIS data model.
Nodes will be translated into points and ways into linestrings or polygons (if
they are closed ways). Multipolygon and boundary relations will be translated
into multipolygons. This transformation is not loss-less, especially
information in non-multipolygon, non-boundary relations is lost.

All tags are preserved in this process. Note that most GIS formats (such as
Shapefiles, etc.) do not support arbitrary tags. Transformation into other GIS
formats will need extra steps mapping tags to a limited list of attributes.
This is outside the scope of this command.

The node locations have to be kept in memory while doings this. Use the
**--index-type**, **-i** option to set the index type used. Default is
`sparse_mmap_array` on Linux and `sparse_mem_array` on OSX/Windows. This is
the right index type for small to medium sized extracts. For large
(continent-sized) extracts or the full planet use `dense_mmap_array` on Linux
or `dense_mem_array` on OSX/Windows.

This program will not work on full history files.


# OPTIONS

-c, --config=FILE
    Read configuration from specified file.

-e, --show-errors
:   Output any geometry errors on STDOUT. This includes ways with a single
    node or areas that can't be assembled from multipolygon relations. This
    output is not suitable for automated use, there are other tools that can
    create very detailed errors reports that are better for that (see
    http://osmcode.org/osm-area-tools/).

-i, --index-type=TYPE
:   Set the index type.

-I, --show-index-types
:   Shows a list of available index types. It depends on your operating system
    which index types are available. The special type `none` is used when
    reading from files with the node locations on the ways.

-n, --keep-untagged
:   If this is set features without any tags will be in the exported data.
    By default these features will be omitted from the output. Tags are the
    OSM tags, not attributes (like id, version, uid, ...) without the tags
    removed by the **exclude_tags** or **include_tags** settings.

-u, --add-unique-id=TYPE
:   Add a unique Id to each feature. TYPE can be either *counter* in which
    case the first feature will get Id 1, the next Id 2 and so on. The type
    of object does not matter in this case. Or the TYPE is *type_id* in which
    case the Id is a string, the first character is the type of object ('n'
    for nodes, 'w' for linestrings created from ways, and 'a' for areas
    created from ways and/or relations, after that there is a unique Id based
    on the original OSM object Id(s).

@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@

# OUTPUT OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name. See the OUTPUT
    FORMATS section for a list of formats.

--fsync
:   Call fsync after writing the output file to force flushing buffers to disk.

-o, --output=FILE
:   Name of the output file. Default is '-' (STDOUT).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.


# CONFIG FILE

The config file is in JSON format. The top-level is an object which contains
the following optional names:

* `attributes`: An object specifying which attributes of OSM objects to export.
   See the ATTRIBUTES section.
* `linear_tags`: An array of expressions specifying tags that should be treated
   as linear. See the FILTER EXPRESSION and AREA HANDLING sections.
* `area_tags`: An array of expressions specifying tags that should be treated
   as area tags. See the FILTER EXPRESSION and AREA HANDLING sections.
* `exclude_tags`: A list of tag expressions. Tags matching these expressions
   are excluded from the output. See the FILTER EXPRESSION section.
* `include_tags`: A list of tag expressions. Tags matching these expressions
   are included in the output. See the FILTER EXPRESSION section.

The `exclude_tags` and `include_tags` options are mutually exclusive. If you
want to just exclude some tags but leave most tags untouched, use the
`exclude_tags` setting. If you only want a defined list of tags, use
`include_tags`.

When no config file is specified, the following settings are used:

@EXPORT_DEFAULT_CONFIG@


# FILTER EXPRESSIONS

A filter expression specifies a tag or tags that should be matched in the data.

Some examples:

amenity
:   Matches all objects with the key "amenity".

highway=primary
:   Matches all objects with the key "highway" and value "primary".

highway!=primary
:   Matches all objects with the key "highway" and a value other than "primary".

type=multipolygon,boundary
:   Matches all objects with key "type" and value "multipolygon" or "boundary".

name,name:de=Kastanienallee,Kastanienstrasse
:   Matches any object with a "name" or "name:de" tag with the value
    "Kastanienallee" or "Kastanienstrasse".

addr:\*
:   Matches all objects with any key starting with "addr:"

name=\*Paris\*
:   Matches all objects with a name that contains the word "Paris".

If there is no equal sign ("=") in the expression only keys are matched and
values can by anything. If there is an equal sign ("=") in the expression, the
key is to the left and the value to the right. An exclamation sign ("!") before
the equal sign means: A tag with that key, but not the value(s) to the right of
the equal sign. A leading or trailing asterisk ("\*") can be used for substring
or prefix matching, respectively. Commas (",") can be used to separate several
keys or values.

All filter expressions are case-sensitive. There is no way to escape the
special characters such as "=", "\*" and ",". You can not mix
comma-expressions and "\*"-expressions.


# ATTRIBUTES

All OSM objects (nodes, ways, and relations) have *attributes*, areas inherit
their attributes from the ways and/or relations they were created from. The
attributes known to `osmium export` are:

* `type` ('node', 'way', or 'relation')
* `id` (64 bit object Id)
* `version` (version number)
* `changeset` (changeset Id)
* `timestamp` (time of object creation in seconds since Jan 1 1970)
* `uid` (user Id)
* `user` (user name)
* `way_nodes` (ways only, array with node Ids)

For areas, the type will be `way` or `relation` if the area was created
from a closed way or a multipolygon or boundary relation, respectively. The
`id` for areas is the id of the closed way or the multipolygon or boundary
relation.

By default the attributes will not be in the export, because they are not
necessary for most uses of OSM data. If you are interested in some (or all)
attributes, add an `attributes` object to the config file. Add a member for
each attribute you are interested in, the value can be either `false` (do not
output this attribute), `true` (output this attribute with the attribute name
prefixed by the `@` sign) or any string, in which case the string will be used
as the attribute name.

Note that the `id` is not necessarily unique. Even the combination `type` and
`id` is  not unique, because a way may end up as LineString and as Polygon
on the file. See the `--add-unique-id` option for a unique Id.


# AREA HANDLING

Multipolygon relations will be assembled into multipolygon geometries forming
areas. Some closed ways will also form areas. Here are the more detailed rules:

* Non-closed ways (last node not the same as the first node) are always
  linestrings, not areas.
* Relations tagged `type=multipolygon` or `type=boundary` are always assembled
  into areas. If they are not valid, they are ignored (but an error message
  will be produced if the --show-errors/-e option is specified).
* For closed ways the tags are checked. If they have an `area` tag other than
  `area=no`, they are areas and a polygon is created. If they have an `area`
  tag other than `area=yes`, they are linestrings. So closed ways can be both,
  an area and a linestring!
* The configuration options `area_tags` and `linear_tags` can be used to
  augment the area check. If any of the tags on a closed way matches any of
  the expressions in `area_tags`, a polygon is created. If any of the tags on
  a closed way matches any of the expressions in `linear_tags`, a linestring
  is created. Again: If both match, an area and a linestring is created.


# OUTPUT FORMATS

The following output formats are supported:

* `geojson` (alias: `json`): GeoJSON. The output file will contain a single
  `FeatureCollection` object. This is the default format.
* `ldgeojson` (alias: `ldjson`): Line delimited GeoJSON. Each line (ending
  in a linefeed character) contains one GeoJSON object. Used for streaming
  GeoJSON.
* `text` (alias: `txt`): A simple text format with the geometry in WKT formats
  followed by the comma-delimited tags. This is mainly intended for debugging
  at the moment. THE FORMAT MIGHT CHANGE WITHOUT NOTICE!


# DIAGNOSTICS

**osmium export** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium export** must keep all node locations and all objects needed for
assembling the areas in memory. For larger data files, this can need several
GBytes of memory. Make sure you are using the right node location store for
your input data.


# EXAMPLES

Export into GeoJSON format:

    osmium export data.osm.pbf -o data.geojson

Use a config file:

    osmium export data.osm.pbf -o data.geojson -c export-config.json


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5), **osmium-add-node-locations-to-ways**(1)
* [Osmium website](http://osmcode.org/osmium-tool/)
* [GeoJSON](http://geojson.org/)
* [Line delimited JSON](https://en.wikipedia.org/wiki/JSON_Streaming#Line_delimited_JSON)

