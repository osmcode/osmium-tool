
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [unreleased] -

### Added

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


[unreleased]: https://github.com/osmcode/osmium-tool/compare/v1.5.0...HEAD
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

