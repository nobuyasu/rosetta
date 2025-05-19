# FreeSASA external library configuration for CMake

# Add FreeSASA include directories
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/src)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa)  # For config.h

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
  # Skip files that require external dependencies
  # ${CMAKE_SOURCE_DIR}/../external/freesasa/src/cif.cc
  # ${CMAKE_SOURCE_DIR}/../external/freesasa/src/json.c
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
  target_compile_definitions(freesasa PRIVATE HAVE_CONFIG_H=1 FREESASA_DISABLE_JSON=1 FREESASA_DISABLE_CIF=1)
  
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