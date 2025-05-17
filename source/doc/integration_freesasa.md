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
# FreeSASA module for CMake build
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../external/freesasa/lib)
```

2. Updated `cmake/build/CMakeLists.txt` to include the module:
```cmake
INCLUDE(modules/freesasa.cmake)
```

3. Updated `cmake/build_release/CMakeLists.txt` to add include paths:
```cmake
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../../external/freesasa/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../../external/freesasa/src)
```

4. Created app-specific `test_freesasa.cmake`:
```cmake
SET( TARGET test_freesasa )
SET( SOURCES apps/pilot/nobuyasu/test_freesasa.cc )
ROSETTA_EXECUTABLE( ${TARGET} ${SOURCES} )
TARGET_LINK_LIBRARIES( test_freesasa 
    ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a
    ${LINK_ALL_LIBS} 
)
```

**Critical insight**: CMake required the full library path, not just `-lfreesasa`.

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

# CMake build  
cd /path/to/rosetta/source
./ninja_build.py r -t test_freesasa
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
   - `cmake/build/modules/freesasa.cmake`
   - Updated `cmake/build/CMakeLists.txt`
   - Updated `cmake/build_release/CMakeLists.txt`
   - App-specific cmake file (e.g., `test_freesasa.cmake`)
4. Correct include paths and compilation flags
5. Understanding of FreeSASA's current API

This integration enables SASA calculations within Rosetta protocols while maintaining compatibility with both build systems.