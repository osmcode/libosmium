
# Change Log

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [unreleased] -

### Added

### Changed

* Changed `add_user()` and `add_role()` in builders to use string length
  without the 0-termination.

### Fixed

## [2.2.0] - 2015-07-04

### Added

- Conversion functions for some low-level types.
- BoolVector index class.
- `min_op`/`max_op` utility functions.
- More tests here and there.
- Helper methods `is_between()` and `is_visible_at()` to DiffObject.
- GeoJSON factory using the RapidJSON library.
- Support for tile calculations.
- Create simple polygons from ways in geom factories.
- `MemoryMapping` and `TypedMemoryMapping` helper classes.
- `close()` function to `mmap_vector_base` class.
- Function on `Buffer` class to get iterator to specific offset.
- Explicit cast operator from `osmium::Timestamp` to `uint32_t`.

### Changed

- Throw exception on illegal values in functions parsing strings to get ids,
  versions, etc.
- Improved error message for geometry exceptions.

### Fixed

- Throw exception from `dump_as_array()` and `dump_as_list()` functions if not
  implemented in an index.
- After writing OSM files, program could stall up to a second.
- Dense location store was written out only partially.
- Use `uint64_t` as counter in benchmarks, so there can be no overflows.
- Example programs now read packed XML files, too.
- Refactoring of memory mapping code. Removes leak on Windows.
- Better check for invalid locations.
- Mark `cbegin()` and `cend()` of `mmap_vector_base` as const functions.

## [2.1.0] - 2015-03-31

### Added

- When writing PBF files, sorting the PBF stringtables is now optional.
- More tests and documentation.

### Changed

- Some functions are now declared `noexcept`.
- XML parser fails now if the top-level element is not `osm` or `osmChange`.

### Fixed

- Race condition in PBF reader.
- Multipolygon collector was accessing non-existent NodeRef.
- Doxygen documentation wan't showing all classes/functions due to a bug in
  Doxygen (up to version 1.8.8). This version contains a workaround to fix
  this.

[unreleased]: https://github.com/osmcode/libosmium/compare/v2.2.0...HEAD
[2.2.0]: https://github.com/osmcode/libosmium/compare/v2.1.0...v2.2.0
[2.1.0]: https://github.com/osmcode/libosmium/compare/v2.0.0...v2.1.0

