
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [unreleased] -

### Added

### Changed

### Fixed


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


[unreleased]: https://github.com/osmcode/osmium-tool/compare/v1.1.0...HEAD
[1.0.1]: https://github.com/osmcode/osmium-tool/compare/v1.0.1...v1.1.0
[1.0.1]: https://github.com/osmcode/osmium-tool/compare/v1.0.0...v1.0.1

