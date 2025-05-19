# FreeSASA Integration Summary

This document summarizes the successful integration of FreeSASA library into Rosetta, highlighting the key insights and solutions discovered through trial and error.

**Last Updated**: January 19, 2025

## Integration Overview

We successfully integrated FreeSASA into Rosetta using the full integration approach, enabling SASA calculations within Rosetta protocols.

## Key Components

### 1. External Library Configuration (SCons)

Created `freesasa.external.settings`:
```python
sources = {
    "freesasa/src": [
        "classifier.c",
        "coord.c",
        "freesasa.c",
        "lexer.c",
        "nb.c",
        "node.c",
        "parser.c",
        "pdb.c",
        "rsa.c",
        "sasa_lr.c",
        "sasa_sr.c",
        "selection.c",
        "structure.c",
        "util.c",
        "log.c",
        "classifier_naccess.c",
        "classifier_oons.c",
        "classifier_protor.c",
    ]
}

include_path = [
    "freesasa/src",
    "freesasa/include",
]

library_path = [
]

libraries = [
]

compileflags = [
    "-DHAVE_CONFIG_H",
]

subprojects = [
]

# Supported platforms
only_with_platforms = ["linux", "macos"]
```

### 2. Build System Configuration

#### SCons Integration

1. Created `external/SConscript.external.freesasa` to register FreeSASA:
```python
Import('external_settings')
external_settings['freesasa'] = ['freesasa.external.settings']
```

2. Updated `pilot_apps.src.settings.all`:
```python
include_path = [
    "#external/freesasa/include",
]
library_path = [
    "#external/freesasa/lib",
]
subprojects.append("freesasa")
```

#### CMake Integration

1. Created `cmake/build/modules/freesasa.cmake`:
```cmake
# FreeSASA external library configuration for CMake
# Add FreeSASA include directory
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/src)

# Add FreeSASA source files
file(GLOB FREESASA_SOURCES 
  "${CMAKE_SOURCE_DIR}/../external/freesasa/src/*.c"
  "${CMAKE_SOURCE_DIR}/../external/freesasa/src/*.cc"
)

# Create FreeSASA library
add_library(freesasa STATIC ${FREESASA_SOURCES})

# Link XML2 library
target_link_libraries(freesasa xml2)

# Add to external libraries
set(LINK_EXTERNAL_LIBS ${LINK_EXTERNAL_LIBS} freesasa)

# Set compilation flags for FreeSASA
target_compile_definitions(freesasa PRIVATE HAVE_CONFIG_H=0)

message(STATUS "FreeSASA library configured for CMake")
```

2. Updated `cmake/build_release/CMakeLists.txt` to add include paths:
```cmake
# Add FreeSASA include paths
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../../external/freesasa/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../../external/freesasa/src)
```

3. Modified app-specific `build/apps/test_freesasa.cmake`:
```cmake
ADD_EXECUTABLE( test_freesasa ../../src/apps/pilot/nobuyasu//test_freesasa.cc )
TARGET_LINK_LIBRARIES( test_freesasa ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a ${LINK_ALL_LIBS} )
SET_TARGET_PROPERTIES( test_freesasa PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}" )
SET_TARGET_PROPERTIES( test_freesasa PROPERTIES LINK_FLAGS "${LINK_FLAGS}" )
ADD_CUSTOM_TARGET( test_freesasa_symlink  ALL)
# cmake -E create_symlink won't choke if the symlink already exists
ADD_CUSTOM_COMMAND( TARGET test_freesasa_symlink POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ../../bin/ 
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/test_freesasa ../../bin/test_freesasa
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/test_freesasa ../../bin/test_freesasa.macos${COMPILER}${MODE}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/test_freesasa ../../bin/test_freesasa.default.macos${COMPILER}${MODE}
)
ADD_DEPENDENCIES( test_freesasa_symlink test_freesasa )
ADD_DEPENDENCIES( test_freesasa BUILD_ROSETTA_LIBS )
ADD_DEPENDENCIES( pilot_apps test_freesasa_symlink )
install(TARGETS test_freesasa RUNTIME DESTINATION bin OPTIONAL)
```

**Critical insight**: CMake required the full library path, not just `-lfreesasa`. The exact path to the static library must be specified in TARGET_LINK_LIBRARIES.

### 3. Application Implementation

Key implementation details:

```cpp
// Correct include path for Rosetta init
#include <core/init/init.hh>

// Hydrogen detection using atom name
for ( Size atom_idx = 1; atom_idx <= residue.natoms(); ++atom_idx ) {
    std::string atom_name = residue.atom_name( atom_idx );
    // Skip hydrogen atoms
    if ( atom_name.size() > 0 && atom_name[0] == 'H' ) continue;
    // ... rest of atom processing
}

// Proper FreeSASA API usage
freesasa_structure* structure = freesasa_structure_new();
if ( !structure ) {
    utility_exit_with_message("Failed to create FreeSASA structure");
}

// Add atoms with proper string conversion
freesasa_structure_add_atom(
    structure,
    atom_name.c_str(),
    res_name.c_str(),
    std::to_string(res_number).c_str(),
    chain_id,
    coords.x(),
    coords.y(),
    coords.z()
);
```

## Critical Discoveries

### 1. Dependency Management
- Disabled XML and JSON support by setting `USE_XML=0` and `USE_JSON=0` in config.h
- Removed `xml.c`, `json.c`, and `cif.cc` from build to avoid external dependencies
- This eliminated the need for json-c and additional libxml2 configuration

### 2. Code Fixes
- Fixed header file return type: Changed `char` to `char*` for `freesasa_node_atom_chain`
- Fixed type errors in structure.c:
  - Changed `freesasa_cif_atom_lcl` to `freesasa_cif_atom`
  - Changed `freesasa_chain_group` to `struct chains`
  - Fixed member access from `chains->chains[i]` to `chains->labels[i]`

### 3. Configuration Files
- Created `config.h` with minimal FreeSASA configuration
- Set `HAVE_CONFIG_H=1` flag for proper compilation
- Used `-DPTR_STD` for Rosetta's smart pointer system

### 4. Build Process

Both SCons and CMake build systems work correctly:

```bash
# SCons build
cd /path/to/rosetta/source
./scons.py mode=release freesasa
./scons.py mode=release test_freesasa

# CMake build with ninja_build.py 
cd /path/to/rosetta/source
./ninja_build.py r -remake
./ninja_build.py r -t test_freesasa

# Clean rebuild for CMake
cd /path/to/rosetta/source/cmake/build_release
ninja clean
rm -rf CMakeFiles CMakeCache.txt *.ninja compile_commands.json
cd ../
rm build/external_freesasa.cmake
./make_project.py all
cd build_release
cmake -G Ninja .
ninja test_freesasa
```

## Testing and Verification

Both build systems produced identical results:
```
Total SASA: 5159.88 Ų
Exposed hydrophobic residues with SASA of at least 20 Ų: 16
```

## Lessons Learned

1. **Build System Differences**: CMake and SCons handle library linking differently. CMake required full library paths while SCons could use standard `-l` notation.

2. **Path Resolution**: Relative paths work better than absolute paths for portability.

3. **API Discovery**: FreeSASA's current API differs from older documentation. Direct experimentation was needed to find correct function signatures.

4. **Rosetta Conventions**: Understanding Rosetta's file organization (core/init/init.hh vs core/init.hh) was crucial.

5. **Simplification**: Many complex solutions in documentation weren't necessary. Simpler approaches often worked better.

## Minimal Requirements

For successful integration, you need:

1. **FreeSASA source files** in `external/freesasa/`
   - Copy only the necessary source files
   - Create a minimal `config.h` file

2. **SCons configuration**:
   - `external/freesasa.external.settings` with the reduced source list
   - `external/SConscript.external.freesasa` (optional, uses default if not present)
   - Update `projects.settings` to include freesasa in external libraries

3. **CMake configuration**:
   - `cmake/build/modules/freesasa.cmake` (optional, for custom configuration)
   - Let `make_project.py` auto-generate `external_freesasa.cmake`
   - App-specific cmake files will use standard linking:
     ```cmake
     TARGET_LINK_LIBRARIES( test_freesasa ${LINK_ALL_LIBS} )
     ```

4. **Code modifications**:
   - Fix type signatures in header files
   - Fix struct member access in implementation files
   - Create proper config.h with disabled optional features

5. **Build flags**:
   - Add `-DHAVE_CONFIG_H` to compile flags
   - Platform-specific flags are handled automatically

This integration enables SASA calculations within Rosetta protocols while maintaining compatibility with both build systems, without requiring external dependencies.

## General Usage of FreeSASA in Rosetta

To make FreeSASA available to all Rosetta applications without requiring explicit linking in each application, we implemented a better approach that follows the standard external library pattern in Rosetta:

### 1. Register FreeSASA as a Standard External Library

Updated `external_libraries.cmake` to include FreeSASA in the standard external libraries list:
```cmake
# Add FreeSASA to the list of external libraries
SET(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} freesasa)
```

### 2. Define FreeSASA Library in external_freesasa.cmake

Created `external_freesasa.cmake` to define the FreeSASA library sources:
```cmake
# FreeSASA external library configuration
# List all source files needed for the FreeSASA library

SET(freesasa_files
	../../external/freesasa/src/classifier.c
	../../external/freesasa/src/classifier_naccess.c
	# Other source files...
	../../external/freesasa/src/json.c
)

# Set any required preprocessor definitions
SET(freesasa_defines "HAVE_CONFIG_H=0")

# Set any specific compiler flags
SET(freesasa_compileflags "")

# Set any specific linker flags
SET(freesasa_linkflags "")
```

### 3. Improved FreeSASA Module Configuration

Enhanced `modules/freesasa.cmake` to directly build and link the FreeSASA library:
```cmake
# FreeSASA external library configuration for CMake

# Add FreeSASA include directories
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/src)

# Define FreeSASA source files directly in this module
set(FREESASA_SOURCES
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/classifier.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/classifier_naccess.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/classifier_oons.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/classifier_protor.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/coord.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/freesasa.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/nb.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/node.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/parser.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/pdb.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/rsa.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/sasa_lr.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/sasa_sr.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/selection.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/structure.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/util.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/lexer.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/xml.c
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/cif.cc
  ${CMAKE_SOURCE_DIR}/../external/freesasa/src/json.c
)

# Check if a pre-built FreeSASA library exists
set(FREESASA_LIB_PATH ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a)

if(EXISTS ${FREESASA_LIB_PATH})
  # Use the pre-built static library directly
  message(STATUS "Found pre-built FreeSASA library at ${FREESASA_LIB_PATH}")
  
  # Add the library directly to LINK_EXTERNAL_LIBS to ensure it's linked with applications
  set(LINK_EXTERNAL_LIBS ${LINK_EXTERNAL_LIBS} ${FREESASA_LIB_PATH})
  message(STATUS "Added pre-built FreeSASA library to LINK_EXTERNAL_LIBS")
else()
  # If the pre-built library doesn't exist, build it from source
  message(STATUS "No pre-built FreeSASA library found. Building from source.")
  
  # Create FreeSASA library from source
  add_library(freesasa STATIC ${FREESASA_SOURCES})
  target_compile_definitions(freesasa PRIVATE HAVE_CONFIG_H=0)
  
  # Add the library to LINK_EXTERNAL_LIBS
  set(LINK_EXTERNAL_LIBS ${LINK_EXTERNAL_LIBS} freesasa)
  message(STATUS "Added FreeSASA built from source to LINK_EXTERNAL_LIBS")
endif()

# Set platform-specific definitions if needed
if(APPLE)
  add_definitions(-DFREESASA_APPLE_PLATFORM)
ELSEIF(UNIX AND NOT APPLE)
  add_definitions(-DFREESASA_LINUX_PLATFORM)
ELSEIF(WIN32)
  add_definitions(-DFREESASA_WINDOWS_PLATFORM)
ENDIF()

message(STATUS "FreeSASA module configured for CMake")
```

### 4. Simplified Application CMake Files

With this approach, applications using FreeSASA don't need to explicitly link to it:
```cmake
ADD_EXECUTABLE( my_app src/path/to/my_app.cc )
# FreeSASA is automatically included via the standard external library system
TARGET_LINK_LIBRARIES( my_app ${LINK_ALL_LIBS} )
```

This approach has several advantages:
- It follows the established pattern for external libraries in Rosetta
- It doesn't require modifying core.3.cmake, which improves maintainability
- It properly handles both pre-built and source-built libraries
- It integrates FreeSASA into the standard build process

All applications linking against the standard Rosetta libraries will automatically have access to FreeSASA functionality.