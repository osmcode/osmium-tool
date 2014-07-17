% OSMIUM(1)
% Jochen Topf <jochen@topf.org>

# NAME
osmium - Multipurpose tool for working with OpenStreetMap data.


# SYNOPSIS

**osmium** *COMMAND* \[ARGS\]\
**osmium** --help\
**osmium** --version


# DESCRIPTION

Multipurpose tool for working with OpenStreetMap data.

Run **osmium help** *COMMAND* to get more information about a command.


# OPTIONS

--help, -h
:   Show usage and list of commands.

--version
:   Show program version.


# COMMANDS

apply-changes
:   Apply OSM change file(s) to OSM data file.
cat
:   Concatenate OSM files and convert to different formats.
fileinfo
:   Show information about an OpenStreetMap file.
help
:   Show help about commands.
merge-changes
:   Merge several OSM change files into one.
time-filter
:   Filter OSM data by time from a history file.


# SEE ALSO

* **osmium-apply-changes**(1), **osmium-cat**(1), **osmium-fileinfo**(1),
  **osmium-merge-changes**(1), **osmium-time-filter**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

