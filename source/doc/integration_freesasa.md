# FreeSASA Integration Summary

This document summarizes the successful integration of FreeSASA library into Rosetta, highlighting the key insights and solutions discovered through trial and error.

## Integration Overview

We successfully integrated FreeSASA into Rosetta using the full integration approach, enabling SASA calculations within Rosetta protocols. The integration works with both SCons and CMake build systems.

## Key Components

### 1. External Library Configuration (SCons)

Created `freesasa.external.settings`:
```python
"freesasa" : {
    "source" : [
        "freesasa/src/classifier.c",
        "freesasa/src/classifier_naccess.c",
        "freesasa/src/classifier_oons.c",
        "freesasa/src/classifier_protor.c",
        "freesasa/src/coord.c",
        "freesasa/src/freesasa.c",
        "freesasa/src/nb.c",
        "freesasa/src/node.c",
        "freesasa/src/parser.c",
        "freesasa/src/pdb.c",
        "freesasa/src/rsa.c",
        "freesasa/src/sasa_lr.c",
        "freesasa/src/sasa_sr.c",
        "freesasa/src/selection.c",
        "freesasa/src/structure.c",
        "freesasa/src/util.c",
        "freesasa/src/lexer.c",
        "freesasa/src/xml.c",
        "freesasa/src/cif.cc",
        "freesasa/src/json.c"
    ],
    "link_suffixes" : [ ""],
    "internal_include_paths" : [
        "freesasa/include"
    ],
    "include_cxxflags" : [
        "-DHAVE_CONFIG_H", "-I./external/freesasa"
    ],
    "exclude_from_static" : False,
}
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

### 1. Path Configuration
- Used relative paths instead of absolute paths for better portability
- Copied FreeSASA files directly into `external/freesasa/` directory
- This approach works across different systems without path modifications

### 2. Compilation Requirements
- Required flag: `-DPTR_STD` for Rosetta's smart pointer system
- C++17 standard: `-std=c++17`
- No need for `-lxml2` dependency in our implementation

### 3. API Usage Patterns
- Hydrogen detection: Check atom name first character (`atom_name[0] == 'H'`)
- Residue number conversion: Use `std::to_string()` instead of complex formatting
- Structure creation: Use `freesasa_structure_new()` and check for NULL

### 4. Build Process
```bash
# SCons build
cd /path/to/rosetta/source
./scons.py -j4 bin mode=release test_freesasa

# CMake build with ninja_build.py 
cd /path/to/rosetta/source
./ninja_build.py r -t test_freesasa

# Alternative CMake build manually
cd /path/to/rosetta/source/cmake
./make_project.py all
cd build_release
cmake -G Ninja
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
1. FreeSASA source files in `external/freesasa/`
2. SCons configuration:
   - `freesasa.external.settings` 
   - `external/SConscript.external.freesasa`
   - Updated `pilot_apps.src.settings.all`
3. CMake configuration:
   - `cmake/build/modules/freesasa.cmake` with FreeSASA library definition
   - Updated `cmake/build_release/CMakeLists.txt` with include paths
   - App-specific cmake file with explicit static library linking:
     ```cmake
     TARGET_LINK_LIBRARIES( test_freesasa ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a ${LINK_ALL_LIBS} )
     ```
4. Correct include paths and compilation flags
5. Understanding of FreeSASA's current API
6. Documentation in CLAUDE.md for future reference

This integration enables SASA calculations within Rosetta protocols while maintaining compatibility with both build systems. The full library path must be specified in CMake configurations for proper linking.

## General Usage of FreeSASA in Rosetta

To make FreeSASA available to all Rosetta applications without requiring explicit linking in each application, we implemented the following solution:

### 1. Link FreeSASA with core.3 Library

Modified `core.3.cmake` to include the FreeSASA library:
```cmake
# Link FreeSASA library directly to core.3
# This ensures all applications linking against core libraries have access to FreeSASA
SET(FREESASA_LIB_PATH ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a)
IF(EXISTS ${FREESASA_LIB_PATH})
  SET(core.3_extra_libs ${FREESASA_LIB_PATH})
  MESSAGE(STATUS "Adding FreeSASA library to core.3 for general use")
ELSE()
  MESSAGE(WARNING "FreeSASA library not found at ${FREESASA_LIB_PATH}")
ENDIF()
```

### 2. Modified Build System to Support Extra Libraries

Updated `build.post.cmake` to utilize the `_extra_libs` variables:
```cmake
FOREACH(LIBRARY ${LIBRARIES})
  # Check if this library has extra libraries defined
  IF(DEFINED ${LIBRARY}_extra_libs)
    TARGET_LINK_LIBRARIES( ${LIBRARY} ${LINK_LAST_LIB} ${${LIBRARY}_extra_libs})
    MESSAGE(STATUS "${LIBRARY} has extra libraries: ${${LIBRARY}_extra_libs}")
  ELSE()
    TARGET_LINK_LIBRARIES( ${LIBRARY} ${LINK_LAST_LIB} )
  ENDIF()
  SET( LINK_ROSETTA_LIBS "${LIBRARY};${LINK_ROSETTA_LIBS}" )
  ADD_DEPENDENCIES( BUILD_ROSETTA_LIBS ${LIBRARY} )
  SET( LINK_LAST_LIB ${LIBRARY} )
ENDFOREACH(LIBRARY)
```

### 3. Simplified Application CMake Files

Now, applications using FreeSASA don't need to explicitly link to it:
```cmake
ADD_EXECUTABLE( my_app src/path/to/my_app.cc )
# FreeSASA is automatically included through core.3
TARGET_LINK_LIBRARIES( my_app ${LINK_ALL_LIBS} )
```

This approach centralizes the FreeSASA dependency in the core.3 library, which is already linked by all applications, eliminating the need for explicit linking in each application's CMake file.