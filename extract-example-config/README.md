
# Example config for creating extracts

These example files show the different ways the extent of extracts to be
created with the `osmium extract` command can be specified.

The config file `extracts.json` contains a list of all extracts that should be
created. In this case several different parts of Germany. You can [download a
Germany extract](https://download.geofabrik.de/europe/germany.html) and then try
this out:

    osmium extract -v -c examples-extract.json germany-latest.osm.pbf

All resulting files will be written to the `/tmp/` directory because of the
`directory` setting in the config file. You can override this on the command
line with the `-d DIR` or `--directory=DIR` option.

