
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

## [unreleased] -

### Added

### Changed

### Fixed


## [1.18.0] - 2025-03-17

### Changed

- If the newest libosmium is used, the `extract` command will now work with
  relation ids that do not fit into 32bit.
- Set header timestamp in `time-filter` output file in some situations.
- Allow GeoJSON extract boundary with only geometry in `extract` command
  instead of a complete feature in the GeoJSON.

### Fixed

- Fixed various issues introduced in 1.17.0 when using the `export` command
  to create GeoJSON files.


## [1.17.0] - 2025-01-14

### Added

- Add options to the "diff" command to ignore changeset, uid, and/or user
  attributes.
- More tests.

### Changed

- Needs at least libosmium version 2.20.0.
- Switch from RapidJSON to NLohman JSON. RapidJSON hasn't seen an update in
  a long time, so we are using a different JSON library.
- Removed "spaten" output format.
- Open writer in some commands earlier, so we see potential errors earlier.
- Lots of small code cleanups.

### Fixed

- Fix reading from STDIN for sort command.
- Make tests using binary files optional, because they don't work with zlib-ng
  that some distributions use. Use `-DRUN_TESTS_WITH_BINARY_COMPARE=OFF` to
  switch off those tests.


## [1.16.0] - 2023-09-20

### Added

- Add "tags" strategy option for smart extract strategy. This checks relations
  for the specified keys/tags. For a relation to be completed it has to match
  the keys/tags.

### Changed

- Limit the number of extracts possible with osmium extract to 500. Prevents
  us from running out of file descriptors.

### Fixed

- Extract with a polygon could fail in some circumstances.
- Compile problem on Windows due to our use of `GetObject()` function from
  RapidJSON.

## [1.15.0] - 2023-01-19

### Changed

- Now requires libosmium version 2.17.0 or newer.
- Remove the long-deprecated `--remove-deleted/--simplify` option on the
  `apply-changes` command.
- Remove the long-deprecated `--omit-rs` option on the `export` command.
- Remove the long-deprecated `--history` option on the `getid` command.
- Update included Catch to v2.13.10.
- Various small code cleanups.

### Fixed

- Fix for polygon extracts. Sometimes objects outside the polygon used for
  extraction could end up in the output.
- Correctly detect relative paths on Windows.
- Fixes cross-compiling for mingw.

## [1.14.0] - 2022-02-07

### Added

- Add `--keep-member-nodes` option to `add-locations-to-ways` command. When
  this is set, all nodes referenced directly from relations are kept in the
  output. If this is used, the input file is read twice an a bit more memory
  is needed. Osmium will run around 15% longer than without this option.
- Add `-S relations=false` option to `complete_ways` extract strategy. If you
  don't need any relations you can set this when using `osmium extract` and
  the extract will be done more quickly.
- New `--clean` option to `osmium extract` command to clean attributes.

### Changed

- Switch to C++14 and CMake 3.5.0 as a minimum requirements.
- Switch to [Catch version 2](https://github.com/catchorg/Catch2/tree/v2.x)
  testing framework.

### Fixed

- Fix possible crash when pager set for `osmium show` command was less then
  4 characters long.
- Fixed hangup in `osmium show` when a non-existing pager was used.
- Fixed some tests.
- Fix/improve error detection in export code. More errors are detected now
  which makes a difference when the `-e` or `-E` options are used.


## [1.13.2] - 2021-10-05

### Added

- New `osmium removeid` command to remove all objects with the specified IDs
  from an OSM file. (This is, in a way, the opposite of the `osmium getid`
  command.)
- New `osmium-output-headers(5)` man page with details about the settings
  supported by the `--output-header` command line option.
- New `-g header.boxes` output option to `osmium fileinfo` to get the
  bounding box(es) set in the header.
- New option `--attributes` for the export command to specified attributes
  that should show up in the output files. (This needed a config file before.)
- New option `--buffer-data` for `osmium cat` command to buffer all read data
  in memory before writing it out again. Can be used for benchmarking.

### Changed

- The `osmium merge` command now checks that the input files are ordered
  correctly and warns or stops if this is not the case.
- Improved messages about files read in verbose mode for `osmium cat`.

### Fixed

- Fixed relation member recursion in tags-filter so that objects referenced
  from matching relations are also in the output.
- Fix point-in-polygon-check in extract command. More nodes directly on the
  boundary should now be inside the polygon. This fixes a problem with extracts
  on the antimeridian.
- When an output file name for the extract command contains multiple dots (.),
  the file type could not always be deduced correctly. This improves detection
  for .json and .poly files.
- Do not show progress when any of the inputs is stdin. Because we don't know
  how large the input file is we can't display a reliable progress indicator.
- Allow `none` index type on `osmium export`.


## [1.13.1] - 2021-02-01

### Changed

- Update `osmium-file-formats` man page to include newer file format options.

### Fixed

- The `extract` command did not work correctly on history files. Sometimes
  deleted objects were not detected as such and the resulting file was not
  a correct history file.
- Open input file only once in `tags-filter` command if `-R`,
  `--omit-referenced` is used. This way it works when reading from STDIN.


## [1.13.0] - 2021-01-08

### Added

- Add support for negative IDs in add-locations-to-ways command.
- Show setting of `-i`, `--invert-match` option on tags-filter command.
- Show bytes read and written in verbose mode in `osmium cat` output.

### Changed

- Now depends on libosmium 2.16.0 or greater.
- Has support for PBF lz4 compression which is enabled by default if the
  library is found. Disable by setting CMake option `WITH_LZ4` to `OFF`.
- An OSM file header can now be set based on the input file header in extract
  command from the config file. Set the header to the special `null` value
  for this.
- The `fileinfo` command now shows a **Metadata** section as described in the
  man page.

### Fixed

- When using the `-I`, `--id-osm-file` option in the `tags-filter` command,
  not only the objects in the specified file were marked but also the nodes
  referenced from ways and the members referenced from relations. This had
  two results: a) When not using `-r`, `--add-referenced`: too many objects
  ended up in the resulting file. b) When using `-t`, `--remove-tags`: tags
  from those member objects were not removed.
- When change files have been created from extracts it is possible that they
  contain objects with the same type, id, version, and timestamp. In that case
  we still want to get the last object available. This is now done correctly.
- Various man page fixes.


## [1.12.1] - 2020-06-27

### Changed

* Require libosmium 2.15.6 which has an important fix related
  to multipolygons assembly (needed for "osmium export").


## [1.12.0] - 2020-04-21

### Added

* New `osmium tags-count` command.
* New `--clean` option to `osmium cat` command to clean attributes.

### Fixed

* Allow index types with file name in export command.

## [1.11.1] - 2019-11-20

### Added

* Introduce a generic facility for setting output format options. They can
  be set on the command line (`--format-option`/`-x`) or in the
  `format_options` section in the config file. Settings can be any
  OPTION=VALUE type string. There are two new settings: For the geojsonseq
  format, the option `print_record_separator=false` replaces the command
  line option `--omit-rs`/`-r` which is now deprecated. The `tags_format`
  option for the Pg output format allows using the `hstore` type for tags
  instead of `json(b)`.

### Changed

* Open output file earlier in tags-filter command, so we see it immediately
  in case this fails.

### Fixed

* When tags-filter is used with `--remove-tags`, matching ways got their
  tags removed if they are also referenced from relations. This was clearly
  wrong.

## [1.11.0] - 2019-09-16

### Added

* New option `--remove-tags`/`-t` to `getid` command. When used the tags of
  all objects are removed that are not explicitly requested but are only
  included to complete references.
* Add `create-locations-index` and `query-locations-index` commands. These
  are used to create, update, query and dump node location indexes on disk.
  These indexes store the location of nodes, typically to add them to ways
  and relations later. It is the same format used by osm2pgsql (they call
  it "flat node store") and by the `add-locations-to-ways` command.
* Support for new [Spaten](https://thomas.skowron.eu/spaten/) export format.
* Add special syntax for `--output-header` to copy header from input.
  Sometimes it is useful to copy header fields from the input to the
  output, for instance the `osmosis_replication_timestamp` field. This
  can now be done for some commands (currently only `extract`) by
  using the special syntax `--output-header=OPTION!`, i.e. using an
  exclamation mark instead of setting a value.

### Changed

* Better checking of coordinates in extract boundary polygons/bboxes.
* Compile with NDEBUG in RelWithDebInfo mode.
* Various code cleanups based on problems found with clang-tidy.
* Updated Catch to version 1.12.2.
* Mark PBF output of extract, renumber, and sort commands as sorted. Uses the
  new header option `sorting` of the libosmium library which is not in a
  released version yet. This sets the `Sort.Type_then_ID` header property
  in the PBF file.

### Fixed

* Only check if way is closed after check that it contains nodes.
* `get_start_id()` is not `noexcept`.
* Man pages correctly show options starting with double dash and other small
  man page fixes.
* Allow file-based location index types (`dense_file_array` and
  `sparse_file_array`) that need a file name. Using them was not possible
  because of an overzealous check that didn't recognize the file name.


## [1.10.0] - 2018-12-10

### Added

* The `fileinfo` command now has an `--object-type`/`-t` option like some
  other commands.
* Extended `fileinfo` command to show internal buffer counts and sizes.
* Add `--strategy` option to `sort` command. New `multipass` strategy which
  reads the input file(s) three times making the sort a bit slower, but also
  using less memory.
* New option `--remove-tags`/`-t` to `tags-filter` command. When used the
  tags of all objects that are not matching the filter expression but are
  included as references are removed.
* New option for smart extract strategy: `complete-partial-relations=X` will
  complete all relations with at least X percent of their members already in
  the extract.
* New export format "pg" creates a file in the PostgreSQL COPY text format
  with the GEOMETRY as WKB and the tags in JSON(B) format. This can be
  imported into a PostgreSQL/PostGIS database very quickly.

### Changed

* Show better error message if output directory is missing for `extract`
  command.

### Fixed

* Several fixes for the `tags-filter` command which could lead to wrong
  results.


## [1.9.1] - 2018-08-18

### Changed

* Improved `export` command man page.

### Fixed

* Regression: Default for `linear_tags` and `area_tags` should be `true`.
  It was before v1.9.0 and it is documented this way.


## [1.9.0] - 2018-08-11

### Added

- Add area matching to `tags-filter`. The `tags-filter` command can now match
  "areas" using the "a/" prefix. Areas in this sense are all closed ways with
  4 or more nodes and all relations with tag "type=multipolygon" or
  "type=boundary".
- Add `--geometry-types` option to the `export` command allowing you to
  restrict the geometry types written out.
- Also print out smallest node, way, and relation ID in `fileinfo` command.
- In the `renumber` command, the start IDs for nodes, ways, and relations
  can now be set together or separately with the `--start-id` option. Negative
  IDs are now also allowed. Also there is a new `--show-index` function that
  prints out the ID mappings in the index.
- More tests and updated documentation.
- Add `-C`, `--print-default-config` option to the `export` command writing
  out the default configuration to stdout.
- New `getparents` command to get the ways used by specified nodes or
  relations having specified members.

### Changed

- Newest version of libosmium (2.14.2) and protozoro (1.6.3) are now required.
- Calculation of CRC32 in the fileinfo command is now optional.
  Calculating the CRC32 is very expensive and going without it makes the
  program much much faster. Use --crc/-c to enable it. It will also be
  automatically enabled for JSON output to stay compatible with earlier
  versions of Osmium which might be used in an automated context (you can
  disable this with `--no-crc`). It is also enabled if `-g data.crc32` is
  specified. If it is enabled we are using the CRC32 implementation from
  zlib which is faster than the one from boost we used before. This is
  possible through some changes in libosmium.
- Treat ways with same node end locations as closed in `export` command
  instead of looking at the IDs. This is more consistent with what the
  libosmium `MultipolygonManager` does.
- In the `export` command, the decision whether a way is treated as a
  linestring or polygon has changed. See the man page for details.
- Create linestring geometries for untagged ways if `-n` or `--keep-untagged`
  option is set. It doesn't matter whether they are closed or not, they are
  only written out as linestrings.

### Fixed

- Show error for ways with less than 2 nodes if --show-errors is set.
- Attributes (such as id, version, timestamp, etc.) can appear in the
  properties of the output with arbitrary configurable names. These could
  overlap with tag keys which we don't want. This change removes tags with
  those keys on the assumption that the names chosen for the attributes
  are sufficiently different (something like "@id") from normal tag keys
  that this will not happen very often and those tags are useless anyway.
- Make `--(no)-progress` option work in `sort` command.


## [1.8.0] - 2018-03-31

### Added

- Support for negative IDs in export command.
- Lots of tests with missing metadata (Thanks to Michael Reichert).
- Add metadata options to the extended output of fileinfo command (Thanks to
  Michael Reichert).
- Add progress bars to many commands.
- Add `--redact` option to the `apply-changes` command to redact (patch)
  history files. The change files can contain any version of any object which
  will replace that version of that object from the input. This allows changing
  the history! This mode is for special use only, for instance to remove
  copyrighted or private data.

### Changed

- Needs libosmium 2.14.0.
- Update included `catch.hpp` to version 1.12.1.
- Removed Makefile. Undocumented and possibly confusing way of building.
  As documented, use CMake directly instead.
- Allow bbox setting with any two opposing corners, instead of insisting on
  bottom-left and top-right corner. This affects the changeset-filter and
  extract commands.
- Allow GeoJSON input file to have a FeatureCollection instead of a Feature.
  Only the first feature of this collection is used.

### Fixed

- Bug in the derive-changes command if it is used without `--keep-details`.
  A deletion of any type of object was written as a deletion of a node.
  (Thanks to Michael Reichert.)
- Fix assertion failure in diff command.
- Throw exception instead of using assert to catch broken rings.
- Disable progress bar if STDOUT isn't a tty.
- Show error when there are no extracts specified in extract command.
- Improve STDIN handling in extract command. STDIN can now be used with the
  `simple` strategy, with other strategies it will give you a nice error
  message.
- Lots of code cleanups based on `clang-tidy` warnings making the code more
  robust.
- Only install manpage directories, not CMake files. (Thanks Bas Couwenberg.)

## [1.7.1] - 2017-08-25

### Added

- Extended some man pages.

### Changed

- Allow any OSM file header option with `fileinfo -g`. There is no final
  list of possible options, so any option should be allowed.
- Needs libosmium 2.13.1.

### Fixed

- Specifying extracts in config files was broken. The `extract` command was
  not reading config files correctly and all resulting OSM files were empty.
  Specifying an extract on the command line using `--bbox` or `--polygon`
  was still working.
- Allow zero-length index files in renumber.


## [1.7.0] - 2017-08-15

### Added

- New `export` command for exporting OSM data into GeoJSON format. The OSM
  data model with its nodes, ways, and relations is very different from the
  data model usually used for geodata with features having point, linestring,
  or polygon geometries. The export command transforms OSM data into a more
  usual GIS data model. Nodes will be translated into points and ways into
  linestrings or polygons (if they are closed ways). Multipolygon and boundary
  relations will be translated into multipolygons. This transformation is not
  loss-less, especially information in non-multipolygon, non-boundary relations
  is lost. All tags are preserved in this process.  Note that most GIS formats
  (such as Shapefiles, etc.) do not support arbitrary tags. Transformation
  into other GIS formats will need extra steps mapping tags to a limited list
  of attributes. This is outside the scope of this command.
- New `--bbox/-B` option to `changeset-filter` command. Only changesets with
  bounding boxes overlapping this bounding box are copied to the output.
- Support for the new `flex_mem` index type for node location indexes. It
  is used by default in the `add_locations_to_ways` and `export` commands.
  The new man page `osmium-index-types` documents this and other available
  indexes.

### Changed

- The order of objects in an OSM file expected by some commands as well as
  the order created by the `sort` command has changed when negative IDs are
  involved. (Negative IDs are sometimes used for objects that have not yet
  been uploaded to the OSM server.) The negative IDs are ordered now before
  the positive ones, both in order of their absolute value. This is the same
  ordering as JOSM uses.
- The commands `check-refs`, `fileinfo`, and `renumber` now also work with
  negative object IDs.
- Allow leading spaces in ID files for `getid` command.
- Various error messages and man pages have been clarified.
- Updated minimum libosmium version required to 2.13.0.
- Update version of Catch unit test framework to 1.9.7.

### Fixed

- Libosmium fix: Changesets with more than 2^16 characters in comments now
  work.
- Libosmium fix: Changeset bounding boxes are now always output to OSM files
  (any format) if at least one of the corners is defined. This is needed to
  handle broken data from the main OSM database which contains such cases.
  This now also works when reading OPL files.


## [1.6.1] - 2017-04-10

### Changed

- Clarify differences between `diff` and `derive-changes` commands in man
  pages.
- Needs current libosmium 2.12.1 now.

### Fixed

- Use empty header for apply-changes instead of the one from input file.
- Call 'less' with -R when using ANSI colors with 'show' command.
- Do not show progress options on show command.


## [1.6.0] - 2017-03-07

### Added

- New `tags-filter` command for filtering OSM files based on tag keys and
  values.
- Add option `--locations-on-ways` to `apply-changes` for updating files
  created with `add-locations-to-ways`.
- Add optional `output_header` on extracts in config file. (#47)
- Add `--change-file-format` to `apply-changes` format for setting format
  of change files.
- Add `--set-bounds` option to `extract` command.

### Changed

- Now requires libosmium 2.12.
- Deprecated `--history` option on `getid` command in favour of
  `--with-history` for consistency with other commands.
- Use new `RelationsMapIndex` from libosmium for `getid` instead of
  `std::multimap`.
- Update included version of Catch unit test framework to 1.8.1 which required
  some changes in the tests.
- Use `osmium::util::file_size` instead of our own `filesize()` function.
- Miscellaneous code cleanups and improved warning messages and man pages.

### Fixed

- Add `-pthread` compiler and linker options on Linux/OSX. This should fix
  a problem where some linker versions will not link binaries correctly when
  the `--as-needed` option is used.
- Typo in GeoJSON parser which broke MultiPolygon support.
- Wrong description of -S option in extract man page.
- Windows build problem related to forced build for old Windows versions.
- All but the first polygon in a GeoJSON multipolygon were ignored by the
  `extract` command.
- Zsh command line completion for some commands.


## [1.5.1] - 2017-01-19

### Changed

- Build with warnings in all build types, not only "Dev".
- Better error messages for command line errors.

### Fixed

- Make `--overwrite` and `--fsync` work in `derive_changes` command.
- A dereference of end iterator in `derive_changes`.
- You can not specify the special file name "-" (to read from STDIN) several
  times for commands reading multiple files.


## [1.5.0] - 2017-01-18

### Added

- New `extract` command to cut out geographical regions from an OSM file
  using a bounding box or (multi)polygon.


## [1.4.1] - 2016-11-20

### Added

- Add hint to error message about `--overwrite` option when trying to open
  an existing file.
- Check the required libosmium version in CMake build.

### Changed

- Add --ignore-missing-nodes to `add-locations-to-ways` subcommand. If this
  is not set, the command will now fail if there are missing nodes needed
  for ways.
- The `check-refs` and `getid` subcommands now use the IdSet class from the
  newest libosmium making them more efficient (especially on very large input
  files).
- Improved error messages for low-level errors.
- Now requires at least libosmium 2.10.2 and protozero 1.4.5.

### Fixed

- Consistently handle `--output-header` option in all commands that create
  standard OSM files.
- Handling of some output options was not correct in `diff` command. They
  do now what is documented and it is documented what they do.
- Progress bar and output from verbose mode will now be kept separate.


## [1.4.0] - 2016-09-15

### Added

- The new manual is a more gentle introduction into the capabilities of
  Osmium Tool. Numerous man page additions.
- New `merge` command to merge any number of sorted OSM files.
- New `derive-changes` command to create change file from two OSM data files.
- New `diff` command to show differences between OSM files.
- The `renumber` command can now optionally only renumber some object types.
- Version information is now printed including the git commit id and always
  shown in verbose mode.
- Added `iwyu` target to CMake config.
- Progress bars will be shown on some commands. (This is configurable at
  run time with the `--progress` and `--no-progress` options.)

### Changed

- The `apply-changes` subcommand now detects whether it is updating a normal
  OSM file or an OSM history file based on file name suffix (can be forced
  with `--with-history`). The options `--simplify` and `--remove-deleted`
  are now deprecated (a warning will be written to stderr). For normal OSM
  files, output is always simplified and deleted objects are removed, for
  OSM history files, all versions of all objects are kept.
- Also check ordering of changesets in `osmium fileinfo -e`.
- The `getid` options `-i` and `-I` can now be used multiple times.
- More consistent warning messages.
- Compiles much faster due to include optimizations.
- Update the included RapidJSON to version 1.1.0.

### Fixed

- CMake now creates `compile_commands.json`.
- Wrapper script now works with quoted arguments.


## [1.3.1] - 2016-06-08

### Added

- Check that input files are correctly ordered in the `renumber` and
  `check-refs` commands.
- Most commands now show the memory used in verbose mode.
- New option `-h`/`--help` on most commands shows command line options.
- New `--default-type` option to getid command.
- The `getid` command can now recursively add all referenced objects.
- New `show` command to quickly see OSM file contents.
- New `add-locations-to-ways` command.

### Changed

- Much faster and more memory efficient implementation of the `renumber`
  command.
- The `getid` command can now read IDs from *stdin*.
- Also show libosmium version when running `version` subcommand.


## [1.3.0] - 2015-11-17

### Added

- Add new `sort` subcommand for sorting OSM data files, history files,
  and change files.
- Add new `changeset-filter` subcommand.
- New option `--fsync` on all subcommands that write an OSM file. Will call
  fsync after writing any file.

### Changed

- Uses new libosmium version now (use at least libosmium version 2.5.3).


## [1.2.1] - 2015-08-31

### Changed

- Uses new libosmium version now (use at least libosmium version 2.4.1).

### Fixed

- Several Windows line ending issues were fixed. Consistently uses files
  in binary mode now with LF line endings.
- CRC calculation fixed in libosmium.


## [1.2.0] - 2015-08-18

### Added

- Added lots of tests.

### Changed

- Uses new libosmium version now (use at least libosmium version 2.3.0).
- Many corrections and updates in manual pages.
- Change CRC32 implementation. The new implementation uses the newly added
  CRC functions from libosmium. This should make the CRC independent of host
  architecture and endianness.
- Lots of refactoring code cleanups.

### Fixed

- Remove license that's not applicable from LICENSE-rapidjson.txt.
- Renumbering didn't work properly in some cases.
- Fix error checking in renumber command.


## [1.1.1] - 2015-07-04

### Fixed

- Osmium fileinfo --show-variables didn't work properly.
- Improved zsh autocompletion


## [1.1.0] - 2015-07-04

### Added

- New getid subcommand to filter by node/way/relation IDs.
- New renumber subcommand to renumber IDs in OSM files.
- New check-refs subcommand to check referential integrity in OSM files.
- Improved testing framework and added some functional tests.
- Fileinfo subcommand can now output a single variable using `-g`.
- Fileinfo subcommand can now output all data in JSON format using the
  RapidJSON library (which is included in the code).
- Fileinfo subcommand now also shows largest ID of each type.

### Changed

- Lots of refactoring to handle command line parsing for different subcommands
  in a uniform manner. Now pretty much all command line options concerning file
  input and output are consistent across subcommand.
- Uses newest libosmium.

### Fixed

- Time-fiter subcommand: Fixed case where objects are updated twice in the
  same second.
- Some corrections in man pages.


## [1.0.1] - 2015-03-31

### Changed

- Minor updates to documentation and build system


[unreleased]: https://github.com/osmcode/osmium-tool/compare/v1.18.0...HEAD
[1.18.0]: https://github.com/osmcode/osmium-tool/compare/v1.17.0...v1.18.0
[1.17.0]: https://github.com/osmcode/osmium-tool/compare/v1.16.0...v1.17.0
[1.16.0]: https://github.com/osmcode/osmium-tool/compare/v1.15.0...v1.16.0
[1.15.0]: https://github.com/osmcode/osmium-tool/compare/v1.14.0...v1.15.0
[1.14.0]: https://github.com/osmcode/osmium-tool/compare/v1.13.2...v1.14.0
[1.13.2]: https://github.com/osmcode/osmium-tool/compare/v1.13.1...v1.13.2
[1.13.1]: https://github.com/osmcode/osmium-tool/compare/v1.13.0...v1.13.1
[1.13.0]: https://github.com/osmcode/osmium-tool/compare/v1.12.1...v1.13.0
[1.12.1]: https://github.com/osmcode/osmium-tool/compare/v1.12.0...v1.12.1
[1.12.0]: https://github.com/osmcode/osmium-tool/compare/v1.11.1...v1.12.0
[1.11.1]: https://github.com/osmcode/osmium-tool/compare/v1.11.0...v1.11.1
[1.11.0]: https://github.com/osmcode/osmium-tool/compare/v1.10.0...v1.11.0
[1.10.0]: https://github.com/osmcode/osmium-tool/compare/v1.9.1...v1.10.0
[1.9.1]: https://github.com/osmcode/osmium-tool/compare/v1.9.0...v1.9.1
[1.9.0]: https://github.com/osmcode/osmium-tool/compare/v1.8.0...v1.9.0
[1.8.0]: https://github.com/osmcode/osmium-tool/compare/v1.7.1...v1.8.0
[1.7.1]: https://github.com/osmcode/osmium-tool/compare/v1.7.0...v1.7.1
[1.7.0]: https://github.com/osmcode/osmium-tool/compare/v1.6.1...v1.7.0
[1.6.1]: https://github.com/osmcode/osmium-tool/compare/v1.6.0...v1.6.1
[1.6.0]: https://github.com/osmcode/osmium-tool/compare/v1.5.1...v1.6.0
[1.5.1]: https://github.com/osmcode/osmium-tool/compare/v1.5.0...v1.5.1
[1.5.0]: https://github.com/osmcode/osmium-tool/compare/v1.4.1...v1.5.0
[1.4.1]: https://github.com/osmcode/osmium-tool/compare/v1.4.0...v1.4.1
[1.4.0]: https://github.com/osmcode/osmium-tool/compare/v1.3.1...v1.4.0
[1.3.1]: https://github.com/osmcode/osmium-tool/compare/v1.3.0...v1.3.1
[1.3.0]: https://github.com/osmcode/osmium-tool/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/osmcode/osmium-tool/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/osmcode/osmium-tool/compare/v1.1.0...v1.2.0
[1.1.1]: https://github.com/osmcode/osmium-tool/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/osmcode/osmium-tool/compare/v1.0.1...v1.1.0
[1.0.1]: https://github.com/osmcode/osmium-tool/compare/v1.0.0...v1.0.1

