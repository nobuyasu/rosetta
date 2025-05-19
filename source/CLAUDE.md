# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### SCons (Traditional Build System)
```bash
# Default debug build
./scons.py bin

# Release build (optimized, ~10x faster)
./scons.py bin mode=release

# Static linking (more portable)
./scons.py bin mode=release extras=static

# Parallel build (N threads)
./scons.py -j8 bin mode=release

# Build specific application
./scons.py bin/benchmark.linuxgccdebug
./scons.py mode=release bin/benchmark.linuxgccrelease

# Restricted build with .my settings
./scons.py bin my
./scons.py bin my_pilot_apps

# Build and run unit tests
./scons.py cat=test
python test/run.py
```

### CMake + Ninja (Modern Build System)
```bash
# Quick build using ninja_build.py wrapper
./ninja_build.py r -remake          # Release build, first time
./ninja_build.py r                  # Subsequent release builds
./ninja_build.py d -remake          # Debug build, first time
./ninja_build.py d                  # Subsequent debug builds

# Build specific targets
./ninja_build.py r -t relax         # Build only relax application
./ninja_build.py d -t unit          # Build only unit tests
./ninja_build.py r -t bin           # Build only executables
./ninja_build.py r -t apps          # Build only public apps
./ninja_build.py r -t pilot_apps    # Build only pilot apps

# Using -my settings
./ninja_build.py r -my -remake      # Use *.src.settings.my files

# Manual CMake + Ninja commands
cd cmake
./make_project.py all
cd build_release
cmake -G Ninja
ninja
```

## Testing Commands

```bash
# Build and run all unit tests
./ninja_build.py d -remake
test/run.py -C -j8 --mute all      # Don't forget the -C flag

# Run specific test suite
test/run.py -C -j8 --mute all core.test

# Run individual test
test/run.py -C -j8 --mute all core.test:ResidueTypeTests
```

## Code Architecture

### Directory Structure
- **`src/`**: Source code
  - **`ObjexxFCL/`**: Fortran compatibility layer (arrays, strings, utilities)
  - **`basic/`**: Core utilities (tracers, options, data caching, I/O)
  - **`numeric/`**: Mathematical utilities (vectors, matrices, geometry)
  - **`utility/`**: General utilities (data structures, string operations)
  - **`core/`**: Core Rosetta functionality (split into numbered modules):
    - **`chemical/`**: Atom types, residue types, chemical definitions
    - **`conformation/`**: Pose structures, residue definitions, geometry
    - **`energy_methods/`**: Scoring function terms
    - **`pose/`**: Pose representation and manipulation
    - **`scoring/`**: Scoring infrastructure and functions
  - **`protocols/`**: Higher-level protocols (also split into numbered modules)
  - **`apps/`**: Applications
    - **`public/`**: Released applications
    - **`pilot/`**: Development applications
- **`external/`**: External libraries (FreeSASA, XML, boost, etc.)
- **`cmake/`**: CMake build configuration
- **`test/`**: Unit tests
- **`scripts/`**: Utility scripts

### Key Concepts
1. **Pose**: Central data structure representing a macromolecular structure
2. **ResidueType**: Chemical definition of residue types
3. **ScoreFunction**: Energy evaluation and scoring
4. **Mover**: Protocol for modifying poses
5. **TaskOperations**: Control side-chain packing
6. **Option System**: Command-line and configuration management

### External Library Integration (e.g., FreeSASA)
For SCons:
1. Create `external/library.external.settings`
2. Create `external/SConscript.external.library`
3. Update appropriate `*.src.settings` files

For CMake:
1. Create `cmake/build/modules/library.cmake`
2. Update CMakeLists.txt to include library paths
3. Link library in application-specific cmake files

## Common Development Tasks

### Adding a New Application
```bash
# Generate boilerplate
cd code_templates
./generate_app_template.py my_app

# Update src.settings files to include the new app
# Build the application
./ninja_build.py r -remake -t my_app
```

### Running Integration Tests
```bash
# Update and run integration tests
cd ../test/integration
python test_local.py -j8
```

### Updating Options System
```bash
# After modifying option definitions
./update_options.sh
```

### Code Generation for Templates
```bash
cd code_templates
./generate_templates.py --help
```

## File Conventions
- Header files: `.hh`
- Forward declarations: `.fwd.hh`
- Implementation files: `.cc`
- Test files: `.cxxtest.hh`
- Settings files: `.src.settings`
- Source settings:
  - `.src.settings.all`: All sources
  - `.src.settings.my`: Personal subset
  - `.src.settings.template`: Template for new developers

## Important Notes
- The codebase uses C++17 standard
- Smart pointers require `-DPTR_STD` flag
- Rosetta uses its own vector classes (`vector1`, etc.) with 1-based indexing
- Follow existing code style and conventions
- Always run unit tests before committing
- Integration tests are in a separate repository