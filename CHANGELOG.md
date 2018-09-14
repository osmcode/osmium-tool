
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

## [unreleased] -

### Added

* The `fileinfo` command now has an `--object-type`/`-t` option like some
  other commands.
* Extended `fileinfo` command to show internal buffer counts and sizes.
* Add `--strategy` option to `sort` command. New `multipass` strategy which
  reads the input file(s) three times making the sort a bit slower, but also
  using less memory.

### Changed

* Show better error message if output directory is missing for `extract`
  command.

### Fixed


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


[unreleased]: https://github.com/osmcode/osmium-tool/compare/v1.9.1...HEAD
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

