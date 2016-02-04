
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [unreleased] -

### Added

- Check that input files are correctly ordered in the renumber and
  check-refs subcommands.
- Most subcommands now show the memory used in verbose mode.

### Changed

### Fixed


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

- Remove license thats not applicable from LICENSE-rapidjson.txt.
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


[unreleased]: https://github.com/osmcode/osmium-tool/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/osmcode/osmium-tool/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/osmcode/osmium-tool/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/osmcode/osmium-tool/compare/v1.1.0...v1.2.0
[1.1.1]: https://github.com/osmcode/osmium-tool/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/osmcode/osmium-tool/compare/v1.0.1...v1.1.0
[1.0.1]: https://github.com/osmcode/osmium-tool/compare/v1.0.0...v1.0.1

