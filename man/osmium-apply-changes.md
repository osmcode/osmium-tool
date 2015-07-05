
# NAME

osmium-apply-changes - apply OSM change file(s) to OSM data file


# SYNOPSIS

**osmium apply-changes** \[*OPTIONS*\] *OSM-DATA-FILE* *OSM-CHANGE-FILE*...


# DESCRIPTION

Merges the content of all OSM change files and applies those changes to the OSM
data file.

Objects in change files will be sorted by type, ID, and version, so it doesn't
matter in what order the change files are given or in what order they contain
the data.

**osmium apply-changes** keeps the contents of the change files in main memory,
so the data has to fit in there!


# OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    See **osmium-file-formats**(5) or the libosmium manual for details.

-F, --input-format=FORMAT
:   The format of the input files. Can be used to set the input format if it
    can't be autodetected from the file names. This will set the format for
    all input files, there is no way to set the format for some input files
    only. See **osmium-file-formats**(5) or the libosmium manual for details.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

-r, --remove-deleted
:   Remove deleted objects from the output. If this is not set, deleted objects
    will be in the output with the visible flag set to false.

-s, --simplify
:   Only write the last version of any object to the output.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium apply-changes** exits with exit code

0
  ~ if everything went alright,
1
  ~ if there was an error processing the data, or
2
  ~ if there was a problem with the command line arguments.


# EXAMPLES

Apply changes in `362.osc.gz` to planet file and write result to `new.osm.pbf`:

    osmium apply-changes --output=new.osm.pbf planet.osm.pbf 362.osc.gz


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5), **osmium-merge-changes**(1)
* [Osmium website](http://osmcode.org/osmium)

