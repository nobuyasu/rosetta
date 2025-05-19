# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building Rosetta

### SCons Build System

For debug build:
```bash
scons bin
```

For release build (optimized):
```bash
scons bin mode=release
```

For specific applications:
```bash
scons bin/app_name.linuxgccdebug  # Debug build for a specific app
scons mode=release bin/app_name.linuxgccrelease  # Release build for a specific app
```

Parallel builds (recommended):
```bash
scons -j8 bin mode=release  # Replace 8 with number of CPU cores
```

### CMake Build System

To build with CMake and ninja:
```bash
cd /path/to/rosetta/source
./ninja_build.py r -t app_name  # Build in release mode
./ninja_build.py d -t app_name  # Build in debug mode
```

Manual CMake build:
```bash
cd /path/to/rosetta/source/cmake
./make_project.py all
cd build_release  # or build_debug
cmake -G Ninja
ninja app_name
```

## Testing

### Running Unit Tests

Build tests and run:
```bash
scons cat=test
python test/run.py
```

To run specific tests:
```bash
python test/run.py --suite=core  # Run core tests
python test/run.py --suite=protocols  # Run protocols tests
python test/run.py --test=numeric.xyzVector  # Run specific test
```

## Integrating External Libraries

When integrating external libraries into Rosetta:

1. Place source files in `external/[library_name]/`
2. Create library configuration in `[library_name].external.settings`
3. Add a reference in `external/SConscript.external.[library_name]`
4. For SCons, update `src/pilot_apps.src.settings.all` to add includes and libraries
5. For CMake, create `cmake/build/modules/[library_name].cmake` and update relevant CMake files

## Rosetta Code Architecture

### Core Components
- **core/chemical**: Defines chemical properties of residues, atoms, and bonds
- **core/conformation**: Manages geometric conformations of structures
- **core/pose**: Central data structure for molecular representation
- **core/scoring**: Energy functions and evaluation
- **core/kinematics**: Handles molecular movement and transformations

### Protocols
- **protocols/moves**: Implements molecular manipulation operations (Movers)
- **protocols/filters**: Evaluation criteria for structures
- **protocols/jd2**: Job distribution system for running protocols
- **protocols/loops**: Functionality for loop modeling

### Common Patterns
- **Pose Objects**: Central data structure representing molecular structures
- **Movers**: Operations that transform a pose
- **ScoreFunctions**: Evaluate the energy/quality of poses
- **Filters**: Test poses against specific criteria
- **Selectors**: Select subsets of residues

## Style Guidelines and Best Practices

- Follow the naming conventions seen in the codebase:
  - Class names are CamelCase
  - Function names are snake_case
  - Member variables with trailing underscore
- Include header guards in all header files
- Follow Rosetta's templating and forward declaration patterns
- Use Tracer for debug output
- Use proper error handling with utility_exit

## Common Command Line Options

Common flags when running Rosetta applications:
```
-in:file:s [PDB file]       # Input structure(s)
-in:file:silent [file]      # Read from silent file
-out:file:silent [file]     # Output to silent file
-out:path [path]            # Output directory
-nstruct [number]           # Number of output structures
-database [path]            # Path to Rosetta database
-ignore_unrecognized_res    # Skip unknown residues in PDB
-overwrite                  # Overwrite existing output files
```

## Adding New Applications

1. Create your application in `src/apps/pilot/your_username/`
2. Use existing applications as templates
3. Applications typically include:
   - Command-line parsing
   - Structure input/output
   - Core algorithm implementation

When adding a new application, ensure it uses Rosetta's standard option parsing system and follows the JD2 job distributor pattern when appropriate.