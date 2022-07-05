
# NAME

osmium-index-types - Index types used to store node locations

# DESCRIPTION

The **osmium add-locations-to-ways** and **osmium export** commands have to
keep an index of the node locations in memory or in a temporary file on disk
while doing their work. There are several different ways this can be done which
have different advantages and disadvantages.

Use the **\--show-index-types/-I** option on these commands to show all
available index types. It depends on your operating system which index types
are available.

Use the **\--index-type/-i** option on these commands to set the index type
to be used.

The default index type is `flex_mem` which will keep all data in memory and
works for small extracts as well as the whole planet file. It is the right
choice for almost all use cases if you have enough memory to keep the whole
index in memory.

For the **osmium export** command, the special type `none` is used when reading
from files with the node locations on the ways. (See
[**osmium-add-node-locations-to-ways**(1)](osmium-add-node-locations-to-ways.html) for how to get a file like this.)

You can use one of the file-based indexes for the node location store to
minimize memory use, but performance will suffer. In this case use
`sparse_file_array` if you have a small or medium sized extract and
`dense_file_array` if you are working with a full planet or a really large
extract.

When using the file-based index types (`*_file_array`), add the filename you
want to use for the index after a comma to the index types like so:

`... -i dense_file_array,index.dat ...`


# MEMORY USE

It depends on the index type used how much memory is needed:

* For `sparse_*_array` types 16 bytes per node in the input file are used.
* For `dense_*_array` types 8 bytes times the largest node ID in the input file
  are used.

The `*_mem_*` types use potentially up to twice this amount.

The `*mem*` and `*mmap*` types store the data in memory, the `*file*` types
in a file on disk.

The `flex_mem` type automatically switches between something similar to
`sparse_mmap_array` for smaller extracts and `dense_mmap_array` for larger
extracts or the whole planet file.

If you specify the **\--verbose/-v** option, Osmium will display how much
memory was used for the index.


# SEE ALSO

* [**osmium**(1)](osmium.html), [**osmium-add-locations-to-ways**(1)](osmium-add-locations-to-ways.html), [**osmium-export**(1)](osmium-export.html)
* [Osmium website](https://osmcode.org/osmium-tool/)
* [Index types](https://osmcode.org/osmium-concepts/#indexes)

