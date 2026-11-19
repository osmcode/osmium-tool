# NAME

osmium-split - split OSM data by object type or into tags and geometry

# SYNOPSIS

**osmium split** \[*OPTIONS*\] *OSM-FILE*

# DESCRIPTION

Splits an OSM file into multiple output files. By default, the input file is
split into three files according to the OSM object types: nodes, ways, and
relations.

If the **--split-details** (**-d**) option is given, the data is split into six
files, separating object tags from geometry and reference lists (coordinates
for nodes, node references for ways, and member lists for relations).

# OPTIONS

-p, --prefix=PREFIX
:   Prefix for the output file names. Default is "split".

-f, --output-format=FORMAT
:   The format of the output files. Default is "osm.pbf". See
    **osmium-file-formats**(5) for a list of supported formats.

-d, --split-details
:   Split into six files (`*_nodes_tags`, `*_nodes_geom`, `*_ways_tags`,
    `*_ways_geom`, `*_relations_tags`, `*_relations_geom`), separating tags
    from geometry and references.

-e, --skip-empty-tags
:   Do not write objects without tags to the `*_tags` files. Only has an effect
    when **--split-details** is used.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@

# DIAGNOSTICS

**osmium split** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.

# EXAMPLES

Split a file into `bayern_nodes.osm.pbf`, `bayern_ways.osm.pbf`, and
`bayern_relations.osm.pbf`:

    osmium split -p bayern bayern-latest.osm.pbf

Split into six files and omit untagged objects from the tag files:

    osmium split -d -e -p bayern bayern-latest.osm.pbf

# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5), **osmium-extract**(1), **osmium-tags-filter**(1)
* [Osmium website](https://osmcode.org)
