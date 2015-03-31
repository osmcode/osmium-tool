
# NAME

osmium-file-formats - OSM file formats known to Osmium

# FORMATS

The **osmium** command line tool supports all file formats supported by the
Osmium library. These are:

* The classical XML format in the variants *.osm* (for data files),
  *.osh* (for data files with history) and *.osc* (for change files).
* The PBF binary format (usually with suffix *.osm.pbf*).
* The OPL format (usually with suffix *.osm.opl*).

In addition, XML and OPL files can be compressed using *gzip* or *bzip2*.
(Add *.gz* or *.bz2* suffixes, respectively.)

# AUTODETECTION

Which format a file has is usually autodetected from the file name suffix.

If this doesn't work, either because you are reading from stdin or writing to
stdout, or because you have an unusual file name, you have to set the format
manually. You can also set the format manually if you want to set special
format options.

Most **osmium** commands support the **--input-format** and **--output-format**
options to set the format. They take a comma-separated list of arguments, the
first is the format, further arguments set additional options.

# SPECIAL FORMAT OPTIONS

The following options can be added when writing OSM files:

xml_change_format=true/false
:   Enable/disable XML change format. Same as *.osc*.

force_visible_flag=true/false (*default: false*)
:   Force writing of visible flag, even for normal OSM XML files.

pbf_dense_nodes=true/false (*default: true*)
:   Enable/disable DenseNodes format for PBF files. 

pbf_compression=true/false (*default: true*)
:   Enable/disable compression in PBF files. Disabling this will make writing
    files a bit faster, but the resulting files are 2 to 3 times bigger.

pbf_add_metadata=true/false (*default: true*)
:   Enable/disable writing of object metadata such as changeset id, username,
    etc. Disabling this will make files a bit smaller.

# EXAMPLES

Here are some examples:

pbf
:   PBF format.
osm.bz2
:   XML format, compressed with bzip2.
osc.gz
:   OSM change file, compressed with gzip.
osm.gz,xml_change_format=true
:   OSM change file, compressed with gzip.


# SEE ALSO

* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

