
# NAME

osmium-merge - merge several sorted OSM files into one


# SYNOPSIS

**osmium merge** \[*OPTIONS*\] *OSM-FILE*...


# DESCRIPTION

Merges the content of all OSM files given on the command line into one large
OSM file. Objects in all files must be sorted by type, ID, and version. The
results will also be sorted in the same way. Objects that appear in multiple
input files will only be in the output once.

If there is only a single input file, its contents will be copied to the
output.

If there are different versions of the same object in the input files, all
versions will appear in the output. So this command will work fine with history
files as input creating a new history file. Do not use this command to merge
non-history files with data from different points in time. It will not work
correctly.

@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

# DIAGNOSTICS

**osmium merge** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium merge** doesn't keep a lot of data in memory, but if you are merging
many files, the buffers might take a noticable amount of memory.


# EXAMPLES

Merge several extracts into one:

    osmium merge washington.pbf oregon.pbf california.pbf -o westcoast.pbf


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5), **osmium-merge-changes**(1)
* [Osmium website](http://osmcode.org/osmium)

