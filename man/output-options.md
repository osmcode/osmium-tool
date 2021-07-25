
# OUTPUT OPTIONS

-f, \--output-format=FORMAT
:   The format of the output file. Can be used to set the output file format
    if it can't be autodetected from the output file name.
    See **osmium-file-formats**(5) or the libosmium manual for details.

\--fsync
:   Call fsync after writing the output file to force flushing buffers to disk.

\--generator=NAME
:   The name and version of the program generating the output file. It will be
    added to the header of the output file. Default is "*osmium/*" and the
    version of osmium.

-o, \--output=FILE
:   Name of the output file. Default is '-' (STDOUT).

-O, \--overwrite
:   Allow an existing output file to be overwritten. Normally **osmium** will
    refuse to write over an existing file.

\--output-header=OPTION=VALUE
:   Add output header option. This command line option can be used multiple
    times for different OPTIONs. See the *osmium-output-headers(5)* man page
    for a list of available header options. For some commands you can use the
    special format "OPTION!" (ie. an exclamation mark after the OPTION and no
    value set) to set the value to the same as in the input file.
