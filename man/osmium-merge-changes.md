
# NAME

osmium-merge-changes - merge several OSM change files into one


# SYNOPSIS

**osmium merge-changes** \[*OPTIONS*\] *OSM-CHANGE-FILE*...


# DESCRIPTION

Merges the content of all change files given on the command line into one large
change file. Objects are sorted by type, ID, and version, so it doesn't matter
in what order the change files are given or in what order they contain the data.


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

--fsync
:   Call fsync after writing the output file to force the OS to flush buffers
    to disk.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the version
    of osmium.

-h, --help
:   Show usage help.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.

-s, --simplify
:   Only write the last version of any object to the output.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# DIAGNOSTICS

**osmium merge-changes** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium merge-changes** keeps the contents of all the change files in main
memory. This will take roughly 10 times as much memory as the files take on
disk in *.osm.bz2* format.


# EXAMPLES

Merge all changes in *changes* directory into *all.osc.gz*:

    osmium merge-changes -o all.osc.gz changes/*.gz

Because `osmium merge-changes` sorts its input, you can also use it to sort
just a single change file:

    osmium merge-changes unsorted.osc.gz -o sorted.osc.gz


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

