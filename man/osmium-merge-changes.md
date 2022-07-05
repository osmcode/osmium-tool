
# NAME

osmium-merge-changes - merge several OSM change files into one


# SYNOPSIS

**osmium merge-changes** \[*OPTIONS*\] *OSM-CHANGE-FILE*...


# DESCRIPTION

Merges the content of all change files given on the command line into one large
change file.

Objects are sorted by type, ID, version, and timestamp so it doesn't matter in
what order the change files are given or in what order they contain the data.
(If you are using change files of extracts this is not necessarily true and you
must specify the change files on the command line in the correct order from
oldest to newest. This is because change files from extracts can contain
multiple different object versions with the same version and timestamp!)

This commands reads its input file(s) only once and writes its output file
in one go so it can be streamed, ie. it can read from STDIN and write to
STDOUT.


# OPTIONS

-s, \--simplify
:   Only write the last version of any object to the output. For an object
    created in one of the change files and removed in a later one, the deleted
    version of the object will still appear because it is the latest version.

@MAN_COMMON_OPTIONS@
@MAN_PROGRESS_OPTIONS@
@MAN_INPUT_OPTIONS@
@MAN_OUTPUT_OPTIONS@

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

* [**osmium**(1)](osmium.html), [**osmium-file-formats**(5)](osmium-file-formats.html), [**osmium-output-headers**(5)](osmium-output-headers.html), [**osmium-merge**(1)](osmium-merge.html)
* [Osmium website](https://osmcode.org/osmium-tool/)

