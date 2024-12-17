
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

The **osmium export** command has to keep an index of the node locations in
memory or in a temporary file on disk while doing its work. There are several
different ways it can do that which have different advantages and
disadvantages. The default is good enough for most cases, but see the
[**osmium-index-types**(5)](osmium-index-types.html) man page for details.

Objects with invalid geometries are silently omitted from the output. This is
the case for ways with less than two nodes or closed ways or relations that
can't be assembled into a valid (multi)polygon. See the options
**\--show-errors/-e** and **\--stop-on-error/-E** for how to modify this
behaviour.

The input file will be read twice (once for the relations, once for nodes and
ways), so this command can not read its input from STDIN.

This command will not work on full history files.

This command will work with negative IDs on OSM objects (for instance on
files created with JOSM).


# OPTIONS

-c, \--config=FILE
:   Read configuration from specified file.

-C, \--print-default-config
:   Print the default config to STDOUT. Useful if you want to change it and
    not write the whole thing manually. If you use this option all other
    options are ignored.

-e, \--show-errors
:   Output any geometry errors on STDERR. This includes ways with a single
    node or areas that can't be assembled from multipolygon relations. This
    output is not suitable for automated use, there are other tools that can
    create very detailed errors reports that are better for that (see
    https://osmcode.org/osm-area-tools/).

-E, \--stop-on-error
:   Usually geometry errors (due to missing node locations or broken polygons)
    are ignored and the features are omitted from the output. If this option
    is set, any error will immediately stop the program.

\--geometry-types=TYPES
:   Specify the geometry types that should be written out. Usually all created
    geometries (points, linestrings, and (multi)polygons) are written to the
    output, but you can restrict the types using this option. TYPES is a
    comma-separated list of the types ("point", "linestring", and "polygon").

-a, \--attributes=ATTRS
:   In addition to tags, also export attributes specified in this comma-separated
    list. By default, none are exported. See the **ATTRIBUTES** section below
    for the known attributes list and an explanation.

-i, \--index-type=TYPE
:   Set the index type. For details see the [**osmium-index-types**(5)](osmium-index-types.html) man
    page.

-I, \--show-index-types
:   Shows a list of available index types. For details see the
    [**osmium-index-types**(5)](osmium-index-types.html) man page. If you use this options all other
    options are ignored.

-n, \--keep-untagged
:   If this is set, features without any tags will be in the exported data.
    By default these features will be omitted from the output. Tags are the
    OSM tags, not attributes (like id, version, uid, ...) without the tags
    removed by the **exclude_tags** or **include_tags** settings.

-u, \--add-unique-id=TYPE
:   Add a unique ID to each feature. TYPE can be either *counter* in which
    case the first feature will get ID 1, the next ID 2 and so on. The type
    of object does not matter in this case. Or the TYPE is *type_id* in which
    case the ID is a string, the first character is the type of object ('n'
    for nodes, 'w' for linestrings created from ways, and 'a' for areas
    created from ways and/or relations, after that there is a unique ID based
    on the original OSM object ID(s). For nodes and ways the numeric part of
    the ID is identical to the original node or way ID. For areas, the ID
    is calculated from the original ID, for ways it is twice the original ID,
    for relations it is twice the original ID plus one.
    If the input file has negative IDs, this can create IDs such as 'w-12'.

-x, \--format-option=OPTION(=VALUE)
:   Set an output format option. The options available depend on the output
    format. See the **OUTPUT FORMAT OPTIONS** section for available options.
    If the VALUE is not set, the OPTION will be set to "true". If needed
    you can specify this option multiple times to set several options. Options
    set on the command line overwrite options set in the config file.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@

# OUTPUT OPTIONS

-f, \--output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name. See the OUTPUT
    FORMATS section for a list of formats.

\--fsync
:   Call fsync after writing the output file to force flushing buffers to disk.

-o, \--output=FILE
:   Name of the output file. Default is '-' (STDOUT).

-O, \--overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.


# CONFIG FILE

The config file is in JSON format. The top-level is an object which contains
the following optional names:

* `attributes`: An object specifying which attributes of OSM objects to export.
   See the ATTRIBUTES section.
* `format_options`: An object specifying output format options. The options
   available depend on the output format. See the **OUTPUT FORMAT OPTIONS**
   section for available options. These options can also be set using the
   command line option **\--format-option/-x**.
* `linear_tags`: An expression specifying tags that should be treated
   as linear tags. See below for details and also look at the AREA HANDLING
   section.
* `area_tags`: An expression specifying tags that should be treated
   as area tags. See below for details and also look at the AREA HANDLING
   section.
* `exclude_tags`: A list of tag expressions. Tags matching these expressions
   are excluded from the output. See the FILTER EXPRESSION section.
* `include_tags`: A list of tag expressions. Tags matching these expressions
   are included in the output. See the FILTER EXPRESSION section.

The `area_tags` and `linear_tags` can have the following values:

true
:   All tags match. (An empty list `[]` can also be used to mean the same,
    but this use is deprecated because it can be confusing.)

false
:   No tags match.

Array
:   The array contains one or more expressions as described in the FILTER
    EXPRESSION section.

null
:   If the `area_tags` or `linear_tags` is set to null or not set at all,
    the inverse of the other setting is used. So if you do not set the
    `linear_tags` but have some expressions in `area_tags`, areas will be
    created for all objects matching those expressions and linestrings for
    everything else. This can be simpler, because you only have to keep
    one list, but in cases where an object can be interpreted as both an
    area and a linestring, only one interpretation will be used.

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

name=\*Paris
:   Matches all objects with a name that contains the word "Paris".

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


# ATTRIBUTES

All OSM objects (nodes, ways, and relations) have *attributes*, areas inherit
their attributes from the ways and/or relations they were created from. The
attributes known to `osmium export` are:

* `type` ('node', 'way', or 'relation')
* `id` (64 bit object ID)
* `version` (version number)
* `changeset` (changeset ID)
* `timestamp` (time of object creation in seconds since Jan 1 1970)
* `uid` (user ID)
* `user` (user name)
* `way_nodes` (ways only, array with node IDs)

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

Another option is to specify attributes list in a comma-separated string
for the **\--attributes/-a** command-line option. This way you cannot
control column names, but also you won't have to create a config file.

Depending on your choice of values for the `attributes` objects, attributes
can have the same name as tag keys. If this is the case, the conflicting tag
is silently dropped. So if there is a tag "@id=foo" and you have set `id` to
`true` in the `attributes` object, the tag will not show up in the output.

Note that the `id` is not necessarily unique. Even the combination `type` and
`id` is  not unique, because a way may end up in the output file as LineString
and as (Multi)Polygon. See the **\--add-unique-id/-u** option for a unique ID.


# AREA HANDLING

Multipolygon relations will be assembled into multipolygon geometries forming
areas. Some closed ways will also form areas. Here are the detailed rules:

Non-closed way
: A non-closed way (with the last node location not the same as the first
  node location) is always (regardless of any tags) a linestring, not an area.

Relation
: A relation tagged `type=multipolygon` or `type=boundary` is always
  (regardless of any tags) assembled into an area.

Closed way
: For a closed way (with the last node location the same as the first node
  location) the tags are checked: If the way has an `area=yes` tag, an
  area is created. If the way has an `area=no` tag, a linestring is created.
  An `area` tag with a value other than `yes` or `no` is ignored. The
  configuration settings `area_tags` and `linear_tags` can be used to
  augment the area check. If any of the tags matches the `area_tags`, an
  area is created. If any of the tags matches the `linear_tags`, a linestring
  is created. If both match, an area and a linestring is created. This is
  important because some objects have tags that make them both, an area and
  a linestring.


# OUTPUT FORMATS

The following output formats are supported:

* `geojson` (alias: `json`): GeoJSON (RFC7946). The output file will contain a
  single `FeatureCollection` object. This is the default format.
* `geojsonseq` (alias: `jsonseq`): GeoJSON Text Sequence (RFC8142). Each line
  (beginning with a RS (0x1e, record separator) and ending in a linefeed
  character) contains one GeoJSON object. Used for streaming GeoJSON.
* `pg`: PostgreSQL COPY text format. One line per object containing the
  WGS84 geometry as WKB, the tags in JSON format and, optionally, more columns
  for id and attributes. You have to create the table manually, then use the
  PostgreSQL COPY command to import the data. Enable verbose output to see
  the SQL commands needed to create the table and load the data.
* `text` (alias: `txt`): A simple text format with the geometry in WKT format
  followed by the comma-delimited tags. This is mainly intended for debugging
  at the moment. THE FORMAT MIGHT CHANGE WITHOUT NOTICE!


# OUTPUT FORMAT OPTIONS

* `print_record_separator` (default: `true`). Set to `false` to not print the
  RS (0x1e, record separator) character when using the GeoJSON Text Sequence
  Format. Ignored for other formats.
* `tags_type` (default: `jsonb`). Set to `hstore` to use HSTORE format
  instead of JSON/JSONB when using the Pg Format. Ignored in other formats.


# DIAGNOSTICS

**osmium export** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium export** will usually keep all node locations and all objects needed
for assembling the areas in memory. For larger data files, this can need
several tens of GBytes of memory. See the [**osmium-index-types**(5)](osmium-index-types.html) man page
for details.


# EXAMPLES

Export into GeoJSON format:

    osmium export data.osm.pbf -o data.geojson

Use a config file and export into GeoJSON Text Sequence format:

    osmium export data.osm.pbf -o data.geojsonseq -c export-config.json


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-index-types**(5)](osmium-index-types.html),
  [**osmium-add-node-locations-to-ways**(1)](osmium-add-node-locations-to-ways.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [GeoJSON](http://geojson.org/)
* [RFC7946](https://tools.ietf.org/html/rfc7946)
* [RFC8142](https://tools.ietf.org/html/rfc8142)
* [Line delimited JSON](https://en.wikipedia.org/wiki/JSON_Streaming#Line_delimited_JSON)
