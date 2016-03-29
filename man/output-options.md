
# OUTPUT OPTIONS

-f, --output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    See **osmium-file-formats**(5) or the libosmium manual for details.

--fsync
:   Call fsync after writing the output file to force flushing buffers to disk.

--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the
    version of osmium.

-o, --output=FILE
:   Name of the output file. Default is '-' (*stdout*).

-O, --overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

--output-header=OPTION
:   Add output header option. This option can be given several times. See the
    *libosmium manual* for a list of allowed header options.
