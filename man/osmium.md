
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

add-locations-to-ways
:   add node locations to ways in OSM file

apply-changes
:   apply OSM change file(s) to OSM data file

cat
:   concatenate OSM files and convert to different formats

changeset-filter
:   filter changesets from OSM changeset files

check-refs
:   check referential integrity of OSM file

derive-changes
:   create OSM change file from two OSM files

diff
:   display differences between OSM files

export
:   export OSM data

extract
:   create geographical extracts from an OSM file

fileinfo
:   show information about an OSM file

getid
:   get objects from OSM file by ID

help
:   show help about commands

merge
:   merge several OSM files into one

merge-changes
:   merge several OSM change files into one

renumber
:   renumber object IDs

show
:   show OSM file

sort
:   sort OSM files

tags-filter
:   filter OSM data based on tags

time-filter
:   filter OSM data by time from a history file


# COMMON OPTIONS

Most commands support the following options:

-h, --help
:   Show short usage information.

-v, --verbose
:   Set verbose mode. The program will output information about what it is
    doing to STDERR.


# MEMORY USAGE

Osmium commands try to do their work as memory efficient as possible. But some
osmium commands still need to load quite a bit of data into main memory. In
some cases this means that only smaller datasets can be handled. Look into the
man pages for the individual commands to learn more about their memory use.

If you use the **-v**, **--verbose** option on most commands they will print
out their peak memory usage at the end. This is the actual amount of memory
used including the program code itself, any needed libraries and the data.
(Printing of memory usage is currently only available on Linux systems.)

If an osmium command exits with an "Out of memory" error, try running it with
**--verbose** on smaller datasets to get an idea how much memory it needs.


# SEE ALSO

* **osmium-add-locations-to-ways**(1),
  **osmium-apply-changes**(1),
  **osmium-cat**(1),
  **osmium-changeset-filter**(1),
  **osmium-check-refs**(1),
  **osmium-derive-changes**(1),
  **osmium-diff**(1),
  **osmium-export**(1),
  **osmium-extract**(1),
  **osmium-fileinfo**(1),
  **osmium-getid**(1),
  **osmium-merge**(1),
  **osmium-merge-changes**(1),
  **osmium-renumber**(1),
  **osmium-show**(1),
  **osmium-sort**(1),
  **osmium-tags-filter**(1),
  **osmium-time-filter**(1),
  **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium-tool/)

