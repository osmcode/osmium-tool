
# NAME

osmium-file-formats - OSM file formats known to Osmium

# FILE TYPES

OSM uses three types of files for its main data:

**Data files**
:   These are the most common files. They contain the OSM data from a specific
    point in time. This can either be a planet file containing *all* OSM data
    or some kind of extract. At most one version of every object (node, way, or
    relation) is contained in this file. Deleted objects are *not* in this
    file. The usual suffix used is `.osm`.

**History files**
:   These files contain not only the current version of an object, but their
    history, too. So for any object (node, way, or relation) there can be zero
    or more versions in this file. Deleted objects can also be in this file.
    The usual suffix used is `.osm` or `.osh`. Because sometimes the same
    suffix is used as for normal data files (`.osm`) and because there is no
    clear indicator in the header, it is not always clear what type of file
    you have in front of you.

**Change files**
:   Sometimes called *diff files* or *replication diffs* these files
    contain the changes between one state of the OSM database and another
    state. Change files can contains several versions of an object and also
    deleted objects. The usual suffix used is `.osc`.

All these files have in common that they contain OSM objects (nodes, ways, and
relations). History files and change files can contain several versions of the
same object and also deleted objects, data files can't.

Where possible, Osmium commands can handle all file types. For some commands
only some file types make sense.

# FORMATS

The **osmium** command line tool supports all major OSM file formats plus
some more. These are:

* The classical XML format in the variants *.osm* (for data files),
  *.osh* (for data files with history) and *.osc* (for change files).
* The PBF binary format (usually with suffix *.osm.pbf* or just *.pbf*).
* The OPL format (usually with suffix *.osm.opl* or just *.opl*).
* The O5M/O5C format (usually with suffix *.o5m* or *.o5c*) (reading only).
* The "debug" format (usually with suffix *.osm.debug*) (writing only).

In addition files in all formats except PBF can be compressed using *gzip* or
*bzip2*. (Add *.gz* or *.bz2* suffixes, respectively.)

# AUTODETECTION

Which format a file has is usually autodetected from the file name suffix.

If this doesn't work, either because you are reading from STDIN or writing to
STDOUT, or because you have an unusual file name, you have to set the format
manually. You can also set the format manually if you want to specify special
format options.

Most **osmium** commands support the **\--input-format/-F** and
**\--output-format/-f** options to set the format. They take a
comma-separated list of arguments, the first is the format, further arguments
set additional options.

# SPECIAL FORMAT OPTIONS

The following options can be added when writing OSM files:

`history`=`true`/`false`
:   Explicitly set whether this file allows multiple versions of the same
    object or not. By default set to `true` for *.osh* and *.osc* files, to
    `false` otherwise.

`xml_change_format`=`true`/`false`
:   Enable/disable XML change format. Same as *.osc*.

`force_visible_flag`=`true`/`false` (*default: false*)
:   Force writing of visible flag, even for normal OSM XML files.

`pbf_dense_nodes`=`true`/`false` (*default: true*)
:   Enable/disable DenseNodes format for PBF files.

`pbf_compression`=`none`/`zlib`/`lz4` (*default: zlib*)
:   Set compression type in PBF files. `zlib` (or `true`) is the default and
    almost all files use this. `none` (or `false`) disables compression which
    will make writing files a bit faster, but the resulting files are 2 to 3
    times bigger. The `lz4` compression is not quite as good as `zlib` but much
    faster to compress and decompress, it is currently not supported by most
    OSM file readers.

`pbf_compression_level`=...
:   Set compression level for PBF. Available values and default depend on the
    compression type used, see the OSM File Formats Manual for details.

`add_metadata`=`true`/`false`/... (*default: true*)
:   Enable/disable writing of object metadata such as changeset id, username,
    etc. Disabling this will make files a bit smaller. This can also be set
    to other values, see the OSM File Formats Manual for details.

`locations_on_ways`=`true`/`false` (*default: false*)
:   Add node locations to way nodes. (PBF, XML, OPL only.)

`use_color`=`true`/`false` (*default: false*)
:   Output with ANSI colors. (DEBUG format only.)

`add_crc32`=`true`/`false` (*default: false*)
:   Add CRC32 checksum to all objects. (DEBUG format only.)


# EXAMPLES

Here are some examples:

`pbf`
:   PBF format.

`pbf,add_metadata=false`
:   PBF format, don't write metadata

`osm.bz2`
:   XML format, compressed with bzip2.

`osc.gz`
:   OSM change file, compressed with gzip.

`osm.gz,xml_change_format=true`
:   OSM change file, compressed with gzip.

`osh.opl`
:   OSM history file in OPL format.


# SEE ALSO

* [**osmium**(1)](osmium.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [OSM File Formats Manual](https://osmcode.org/file-formats-manual/)

