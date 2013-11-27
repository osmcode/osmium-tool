% OSMIUM-FILEINFO(1)
% Jochen Topf <jochen@topf.org>

# NAME

osmium-fileinfo - Show information about an OpenStreetMap file.


# SYNOPSIS

**osmium fileinfo** \[OPTIONS\] *FILE*


# DESCRIPTION

Shows verious information about OSM files such as the file type, bounding box,
etc.

This command will usually only read the file header. Use the **--extended**
option to show more information.

The output is split into three sections:

File
:   This section shows the information available without opening the
    file itself. It contains the file name, the format, the compression
    used and the size of the file in bytes.

Header
:   This section shows the information available from the header of
    the file (if available, OPL files have no header).

Data
:   This section shows the information available from reading the whole
    file. It is only shown if the **--extended** option was used.


# OPTIONS

--extended, -e
:   Read the complete file and show additional information.

--input-format, -f
:   The format of the input file. Can be used to set the input file format
    if it can't be autodetected from the file name. See the FILE FORMATS
    section of the osmium-cat man page for details.


# COPYRIGHT

Copyright (C) 2013  Jochen Topf <jochen@topf.org>.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.


# SEE ALSO

<http://osmcode.org/osmium>

