
# NAME
osmium - multipurpose tool for working with OpenStreetMap data


# SYNOPSIS

**osmium** *COMMAND* \[*ARG*...\]\
**osmium** --version


# DESCRIPTION

Multipurpose tool for working with OpenStreetMap data.

Run **osmium help** *COMMAND* to get more information about a command.


# OPTIONS

-h, --help
:   Show usage and list of commands.

--version
:   Show program version.


# COMMANDS

apply-changes
:   apply OSM change file(s) to OSM data file

cat
:   concatenate OSM files and convert to different formats

changeset-filter
:   filter changesets from OSM changeset files

check-refs
:   check referential integrity of OSM file

fileinfo
:   show information about an OSM file

getid
:   get objects from OSM file by ID

help
:   show help about commands

merge-changes
:   merge several OSM change files into one

renumber
:   renumber object IDs

sort
:   sort OSM files

time-filter
:   filter OSM data by time from a history file


# COMMON OPTIONS

Most commands support the following options:

-h, --help
:   Show short usage information.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to *stderr*.


# MEMORY USAGE

Osmium commands try to do their work as memory efficient as possible. But some
osmium commands still need to load quite a bit of data into main memory. In
some cases this means that only smaller datasets can be handled. Look into the
man pages for the individual commands to learn more about their memory use.

If you use the **-v**, **--verbose** option on most commands they will print
out their peak memory usage at the end. This is the actual amount of memory
used including the program code itself, any needed libraries and the data.

If an osmium command exits with an "Out of memory" error, try running it with
**--verbose** on smaller datasets to get an idea how much memory it needs.


# SEE ALSO

* **osmium-apply-changes**(1),
  **osmium-cat**(1),
  **osmium-check-refs**(1),
  **osmium-fileinfo**(1),
  **osmium-getid**(1),
  **osmium-merge-changes**(1),
  **osmium-renumber**(1),
  **osmium-time-filter**(1),
  **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

