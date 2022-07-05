
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

If you have objects with the same type, id, and version but different other
data, the result of this command is undefined. This situation can never happen
in correct OSM files, but sometimes buggy programs can generate data like this.
Osmium doesn't make any promises on what the result of the command is if the
input data is not correct.

This commands reads its input file(s) only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.

# OPTIONS

-H, \--with-history
:   Do not warn when there are multiple versions of the same object in the
    input files.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
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
many files, the buffers might take a noticeable amount of memory.


# EXAMPLES

Merge several extracts into one:

    osmium merge washington.pbf oregon.pbf california.pbf -o westcoast.pbf


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html), [**osmium-merge-changes**(1)](osmium-merge-changes.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

