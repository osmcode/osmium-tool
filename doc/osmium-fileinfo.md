% OSMIUM-FILEINFO(1)
% Jochen Topf <jochen@topf.org>

# NAME

osmium-fileinfo - Show information about an OpenStreetMap file.


# SYNOPSIS

**osmium fileinfo** \[OPTIONS\] *FILE*


# DESCRIPTION

Shows various information about OSM files such as the file type, bounding boxes
in the header, etc.

This command will usually only read the file header. Use the **--extended**
option to show more information.

The output is split into three sections:

File
:   This section shows the information available without opening the
    file itself. It contains the file name, the format deduced from the
    file name, the compression used and the size of the file in bytes.

Header
:   This section shows the information available from the header of
    the file (if available, OPL files have no header). Any available
    bounding boxes are shown as well as header options such as the
    generator and file format version.

Data
:   This section shows the information available from reading the whole
    file. It is only shown if the **--extended** option was used. It
    shows the actual bounding box calculated from the nodes in the file,
    the first and last timestamp of all objects in the file, a SHA1
    checksum of the data in the file, the number of changesets, nodes,
    ways, and relations found in the file, whether the objects in the
    file were ordered by type (nodes, then ways, then relations) and
    id, and whether there were multiple versions of the same object in
    the file (history files and change files can have that).


# OPTIONS

--extended, -e
:   Read the complete file and show additional information.

--input-format, -f
:   The format of the input file. Can be used to set the input file format
    if it can't be autodetected from the file name.
    See **osmium-file-formats**(5) or the libosmium manual for details.


# DIAGNOSTICS

**osmium fileinfo** exits with code 0 if everything went alright, it exits
with code 2 if there was a problem with the command line arguments,
and with exit code 1 if some other error occurred.


# NOTES

Calculation of the SHA1 is only available when Osmium was compiled with support
for it. Check **osmium --version** to see whether thats the case.


# SEE ALSO

* [Osmium website](http://osmcode.org/osmium)
* [Libosmium manual](http://osmcode.org/libosmium/manual/libosmium-manual.html)

