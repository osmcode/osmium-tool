
# NAME

osmium-extract - create geographical extracts from an OSM file


# SYNOPSIS

**osmium extract** -c *CONFIG-FILE* \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Create geographical extracts from an OSM data file or an OSM history file.
The region (geographical extent) can be given as a bounding box or as a
(multi)polygon. The command can create several extracts at the same time. You
have to supply a config file which contains (at least) the output file names
and regions. See the **CONFIG FILE** section for details.

The input file is assumed to be ordered in the usual order: nodes first, then
ways, then relations.

If the `--with-history` option is used, the command will work correctly for
history files. This currently works for the **complete_ways** strategy only.
The **simple** or **smart** strategies do not work with history files. A
history extract will contain every version of all objects with at least one
version in the region. Generating a history extract is somewhat slower than
a normal data extract.

No **bounds** will be set in the header of the output file. Which bounds would
be correct is unclear and setting it correctly might need an extra pass through
the input file.


# OPTIONS

-c, --config=FILE
:   Set the name of the config file.

-d, --directory=DIRECTORY
:   Output directory. Output file names in the config file are relative to
    this directory. Overwrites the setting of the same name in the config
    file.

-s, --strategy=STRATEGY
:   Use the given strategy to extract region. For possible values and details
    see under STRATEGIES. Default is "complete_ways".

-S, --option=OPTION=VALUE
:   Set a named option for the strategy. If needed you can specify this
    option multiple times to set several options.

--with-history
:   Specify that the input file is a history file. The output files will also
    be history files.


@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@

--fsync
:   Call fsync after writing the output file to force flushing buffers to disk.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the
    version of osmium.

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.


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
with documenting the file contents.

    "extracts": [
        {
            "output": "hamburg.osm.pbf",
            "description": "optional description",
            "box": ...
        },
        {
            "output": "berlin.osm.pbf",
            "description": "optional description",
            "polygon": ...
        },
        {
            "output": "munich.osm.pbf",
            "description": "optional description",
            "multipolygon": ...
        }
    ]

There are several formats for specifying the regions:

**box**:

A bounding box in one of two formats. The first is a simple array with
four real numbers specifying the left, bottom, right, and top boundary
(in that order):

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


# (MULTI)POLYGON FILE FORMATS

External files describing a (multi)polygon are specified in the config file
using the "file_name" and "file_type" properties on the "polygon" or
"multipolygon" object:

    "polygon": {
        "file_name": "berlin.geojson",
        "file_type": "geojson"
    }

If file names don't start with a slash (/), they are interpreted relative to
the directory where the config file is.

The following file types are supported:

geojson
:   GeoJSON file containing exactly one polygon or multipolygon. Everything
    except the actual geometry is ignored.

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
    http://www.openstreetmap.org/api/0.6/relation/RELATION-ID/full . Note
    that this can get you very detailed boundaries which can take quite a
    while to cut out. Consider simplifying the boundary before use.

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

For the **smart** strategy you can change the types of relations that should be
reference-complete. Instead of just relations tagged "type=multipolygon", you
can either get all relations (use "-S all") or give a list of types to the
-S option: "-S multipolygon,route". Note that especially boundary relations
can be huge, so if you include them, be aware your result might be huge.


# DIAGNOSTICS

**osmium extract** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments, config file or
    geometry files.


# MEMORY USAGE

Memory usage of **osmium extract** depends on the number of extracts and on the
strategy used. For the *simple* strategy it will at least be the number of
extracts times the highest node ID used devided by 8. For the *complete_ways*
twice that and for the *smart* strategy a bit more.


# EXAMPLES

See the example config files in the *extract-example-config* directory. To
try it:

    osmium extract -v -c extract-example-config/extracts.json \
        germany-latest.osm.pbf


# SEE ALSO

* **osmium**(1), **osmium-merge**(1)
* [Osmium website](http://osmcode.org/osmium-tool/)

