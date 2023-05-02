
# NAME

osmium-extract - create geographical extracts from an OSM file


# SYNOPSIS

**osmium extract** \--config *CONFIG-FILE* \[*OPTIONS*\] *OSM-FILE*\
**osmium extract** \--bbox *LEFT*,*BOTTOM*,*RIGHT*,*TOP* \[*OPTIONS*\] *OSM-FILE*\
**osmium extract** \--polygon *POLYGON-FILE* \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Create geographical extracts from an OSM data file or an OSM history file.
The region (geographical extent) can be given as a bounding box or as a
(multi)polygon.

There are three ways of calling this command:

* Specify a config file with the **\--config/-c** option. It can define any number
  of regions you want to cut out. See the **CONFIG FILE** section for details.

* Specify a bounding box to cut out with the **\--bbox/-b** option.

* Specify a (multi)polygon to cut out with the **\--polygon/-p** option.

The input file is assumed to be ordered in the usual order: nodes first, then
ways, then relations.

If the **\--with-history/-H** option is used, the command will work correctly for
history files. This currently works for the **complete_ways** strategy only.
The **simple** or **smart** strategies do not work with history files. A
history extract will contain every version of all objects with at least one
version in the region. Generating a history extract is somewhat slower than
a normal data extract.

Osmium will make sure that all nodes on the vertices of the boundary of the
region will be in the extract, but nodes that happen to be directly on the
boundary, but between those vertices, might end up in the extract or not. In
almost all cases this will be good enough, but if you want to make really sure
you got everything, use a small buffer around your region.

By default no **bounds** will be set in the header of the output file. Use
the **\--set-bounds** option if you need this.

Note that **osmium extract** will never clip any OSM objects, ie. it will not
remove node references outside the region from ways or unused relation members
from relations. This means you might get objects that are not
reference-complete. It has the advantage that you can use **osmium merge**
to merge several extracts without problems.


# OPTIONS

-b, \--bbox=LONG1,LAT1,LONG2,LAT2
:   Set the bounding box to cut out. Can not be used with **\--polygon/-p**,
    **\--config/-c**, or **\--directory/-d**. The coordinates LONG1,LAT1 are
    from one arbitrary corner, the coordinates LONG2,LAT2 are from the opposite
    corner.

-c, \--config=FILE
:   Set the name of the config file. Can not be used with the **\--bbox/-b** or
    **\--polygon/-p** option. If this is set, the **\--output/-o** and
    **\--output-format/-f** options are ignored, because they are set in the
    config file.

\--clean=ATTR
:   Clean the attribute (*version*, *timestamp*, *changeset*, *uid*, *user*),
    from the data before writing it out again. The attribute will be set to 0
    (the user will be set to the empty string). This option can be given
    multiple times. Depending on the output format these attributes might
    show up as 0 or not show up at all.

-d, \--directory=DIRECTORY
:   Output directory. Output file names in the config file are relative to
    this directory. Overwrites the setting of the same name in the config
    file. This option is ignored when the **\--bbox/-b** or **\--polygon/-p**
    options are used, set the output directory and name with the
    **\--output/-o** option in that case.

-H, \--with-history
:   Specify that the input file is a history file. The output file(s) will also
    be history file(s).

-p, \--polygon=POLYGON_FILE
:   Set the polygon to cut out based on the contents of the file. The file
    has to be a GeoJSON, poly, or OSM file as described in the
    **(MULTI)POLYGON FILE FORMATS** section. It has to have the right suffix
    to be detected correctly. Can not be used with **\--bbox/-b**,
    **\--config/-c**, or **\--directory/-d**.

-s, \--strategy=STRATEGY
:   Use the given strategy to extract the region. For possible values and
    details see the **STRATEGIES** section. Default is "complete_ways".

-S, \--option=OPTION=VALUE
:   Set a named option for the strategy. If needed you can specify this
    option multiple times to set several options.

\--set-bounds
:   Set the bounds field in the header. The bounds are set to the bbox or
    envelope of the polygon specified for the extract. Note that strategies
    other than "simple" can put nodes outside those bounds into the output
    file.


@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@


# CONFIG FILE

The config file mainly specifies the file names and the regions of the extracts
that should be created.

The config file is in JSON format. The top-level is an object which contains at
least an "extracts" array. It can also contain a "directory" entry which names
the directory where all the output files will be created:

    {
        "extracts": [...],
        "directory": "/tmp/"
    }


The extracts array specifies the extracts that should be created. Each item in
the array is an object with at least a name "output" naming the output file and
a region defined in a "bbox", "polygon" or "multipolygon" name. An optional
"description" can be added, it will not be used by the program but can help
with documenting the file contents. You can add an optional "output_format"
if the format can not be detected from the "output" file name. Run "osmium
help file-formats" to get a description of allowed formats.

The optional "output_header" allows you to set additional OSM file header
settings such as the "generator". If you set the value of a file header setting
to `null`, the output header will be set to the same header from the input
file.

    "extracts": [
        {
            "output": "hamburg.osm.pbf",
            "output_format": "pbf",
            "description": "optional description",
            "bbox": ...
        },
        {
            "output": "berlin.osm.pbf",
            "description": "optional description",
            "polygon": ...
        },
        {
            "output": "munich.osm.pbf",
            "output_header": {
                "generator": "MyExtractor/1.0",
                "osmosis_replication_timestamp": null
            },
            "description": "optional description",
            "multipolygon": ...
        }
    ]

There are several formats for specifying the regions:

**bbox**:

A bounding box in one of two formats. The first is a simple array with
four real numbers, the first two specifying the coordinates of an arbitrary
corner, the second two specifying the coordinates of the opposite corner.

    {
        "output": "munich.osm.pbf",
        "description": "Bounding box specified in array format",
        "bbox": [11.35, 48.05, 11.73, 48.25]
    }

The second format uses an object instead of an array:

    {
        "output": "dresden.osm.pbf",
        "description": "Bounding box specified in object format",
        "bbox": {
            "left": 13.57,
            "right": 13.97,
            "top": 51.18,
            "bottom": 50.97
        }
    }

**polygon**:

A polygon, either specified inline in the config file or read from an external
file. See the **(MULTI)POLYGON FILE FORMATS** section for external files. If
specified inline this is a nested array, the outer array defining the polygon,
the next array the rings and the innermost arrays the coordinates. This format
is the same as in GeoJSON files.

In this example there is only one outer ring:

    "polygon": [[
        [9.613465, 53.58071],
        [9.647599, 53.59655],
        [9.649288, 53.61059],
        [9.613465, 53.58071]
    ]]

In each ring, the last set of coordinates should be the same as the first set,
closing the ring.

**multipolygon**:

A multipolygon, either specified inline in the config file or read from an
external file. See the **(MULTI)POLYGON FILE FORMATS** section for external
files. If specified inline this is a nested array, the outer array defining the
multipolygon, the next array the polygons, the next the rings and the innermost
arrays the coordinates. This format is the same as in GeoJSON files.

In this example there is one outer and one inner ring:

    "multipolygon": [[[
        [6.847, 50.987],
        [6.910, 51.007],
        [7.037, 50.953],
        [6.967, 50.880],
        [6.842, 50.925],
        [6.847, 50.987]
    ],[
        [6.967, 50.954],
        [6.969, 50.920],
        [6.932, 50.928],
        [6.934, 50.950],
        [6.967, 50.954]
    ]]]

In each ring, the last set of coordinates should be the same as the first set,
closing the ring.

Osmium must check each and every node in the input data and find out in which
bounding boxes or (multi)polygons this node is. This is very cheap for bounding
boxes, but more expensive for (multi)polygons. And it becomes more expensive
the more vertices the (multi)polyon has. Use bounding boxes or simplified
polygons where possible.

Note that bounding boxes or (multi)polygons are not allowed to span the
-180/180 degree line. If you need this, cut out the regions on each side and
use **osmium merge** to join the resulting files.


# (MULTI)POLYGON FILE FORMATS

External files describing a (multi)polygon are specified in the config file
using the "file_name" and "file_type" properties on the "polygon" or
"multipolygon" object:

    "polygon": {
        "file_name": "berlin.geojson",
        "file_type": "geojson"
    }

If file names don't start with a slash (/), they are interpreted relative to
the directory where the config file is. If the "file_type" is missing, Osmium
will try to autodetect it from the suffix of the "file_name".

The following file types are supported:

geojson
:   GeoJSON file containing exactly one Feature of type Polygon or
    MultiPolygon, or a FeatureCollection with the first Feature of type
    Polygon or MultiPolygon. Everything except the actual geometry (of the
    first Feature) is ignored.

poly
:   A poly file as described in
    https://wiki.openstreetmap.org/wiki/Osmosis/Polygon_Filter_File_Format .
    This wiki page also mentions several sources for such poly files.

osm
:   An OSM file containing one or more multipolygon or boundary relation
    together with all the nodes and ways needed. Any OSM file format (XML,
    PBF, ...) supported by Osmium can be used here, but the correct suffix
    must be used, so the file format is detected correctly. Files for this can
    easily be obtained by searching for the area on OSM and then downloading
    the full relation using a URL like
    https://www.openstreetmap.org/api/0.6/relation/RELATION-ID/full . Or
    you can use **osmium getid -r** to get a specific relation from an OSM
    file. Note that both these approaches can get you very detailed boundaries
    which can take quite a while to cut out. Consider simplifying the boundary
    before use.

If there are several (multi)polygons in a poly file or OSM file, they will
be merged. The (multi)polygons must not overlap, otherwise the result is
undefined.


# STRATEGIES

**osmium extract** can use different strategies for creating the extracts.
Depending on the strategy different objects will end up in the extracts. The
strategies differ in how much memory they need and how often they need to read
the input file. The choice of strategy depends on how you want to use the
generated extracts and how much memory and time you have.

The default strategy is **complete_ways**.

Strategy **simple**
:   Runs in a single pass. The extract will contain all nodes inside the
    region and all ways referencing those nodes as well as all relations
    referencing any nodes or ways already included. Ways crossing the region
    boundary will not be reference-complete. Relations will not be
    reference-complete. This strategy is fast, because it reads the input only
    once, but the result is not enough for most use cases. It is the only
    strategy that will work when reading from a socket or pipe. This strategy
    will not work for history files.

Strategy **complete_ways**
:   Runs in two passes. The extract will contain all nodes inside the region
    and all ways referencing those nodes as well as all nodes referenced by
    those ways. The extract will also contain all relations referenced by
    nodes inside the region or ways already included and, recursively, their
    parent relations. The ways are reference-complete, but the relations are
    not.

Strategy **smart**
:   Runs in three passes. The extract will contain all nodes inside the region
    and all ways referencing those nodes as well as all nodes referenced by
    those ways. The extract will also contain all relations referenced by
    nodes inside the region or ways already included and, recursively, their
    parent relations. The extract will also contain all nodes and ways (and
    the nodes they reference) referenced by relations tagged
    "type=multipolygon" directly referencing any nodes in the region or ways
    referencing nodes in the region. The ways are reference-complete, and
    all multipolygon relations referencing nodes in the regions or ways that
    have nodes in the region are reference-complete. Other relations are not
    reference-complete.

For the **complete_ways** strategy you can set the option "-S relations=false"
in which case no relations will be written to the output file.

The **smart** strategy allows the following strategy options:

Use "-S types=TYPE,..." to change the types of relations that should be
reference-complete. Instead of just relations tagged "type=multipolygon", you
can either get all relations (use "-S types=any") or give a list of types to
the -S option: "-S types=multipolygon,route". Note that especially boundary
relations can be huge, so if you include them, be aware your result might be
huge.

Use "-S complete-partial-relations=X" to force completion of partly completed
relations. If this is set, all relations that have more than X percent of their
members already in the extract will have their full set of members in the
extract. So this allows completing almost complete relations. It can be useful
for instance to make sure a boundary relation is complete even if some of it is
outside the polygon used for extraction.

Use "-S tags=PATTERN,..." to only complete relations that have a tag matching
one of the PATTERNs. So for example if you use "-S
tags=landuse,natural=wood,natural=water" everything tagged `landuse=*` or
`natural=wood` or `natural=water` is added to the result, but no other
relations.

You can combine the "-S types", "-S complete-partial-relations", and "-S tags"
options. The options will be interpreted as "(types OR
complete-partial-relations) AND tags".

# DIAGNOSTICS

**osmium extract** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments, config file or
    polygon files.


# MEMORY USAGE

Memory usage of **osmium extract** depends on the number of extracts and on the
strategy used. For the *simple* strategy it will at least be the number of
extracts times the highest node ID used divided by 8. For the *complete_ways*
twice that and for the *smart* strategy a bit more.

If you want to split a large file into many extracts, do this in several
steps. First create several larger extracts and then split them again and
again into smaller pieces.


# LIMITS

You can not have more than 500 extracts. Although chances are that you will be
running out of memory long before that. See MEMORY USAGE.


# EXAMPLES

See the example config files in the *extract-example-config* directory. To
try it:

    osmium extract -v -c extract-example-config/extracts.json \
        germany-latest.osm.pbf

Extract the city of Karlsruhe using a boundary polygon:

    osmium extract -p karlsruhe-boundary.osm.bz2 germany-latest.osm.pbf \
        -o karlsruhe.osm.pbf

Extract the city of Munich using a bounding box:

    osmium extract -b 11.35,48.05,11.73,48.25 germany-latest.osm.pbf \
        -o munich.osm.pbf


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html),
  [**osmium-getid**(1)](osmium-getid.html), [**osmium-merge**(1)](osmium-merge.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

