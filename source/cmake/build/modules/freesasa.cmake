# FreeSASA external library configuration for CMake

# Add FreeSASA include directories
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/src)

# Check if a pre-built FreeSASA library exists
set(FREESASA_LIB_PATH ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a)

if(EXISTS ${FREESASA_LIB_PATH})
  # Use the pre-built static library directly
  message(STATUS "Found pre-built FreeSASA library at ${FREESASA_LIB_PATH}")
  
  # When using the external library system, we do NOT need to manually add the library
  # to LINK_EXTERNAL_LIBS as it will be handled by external.cmake
  message(STATUS "FreeSASA will be linked through the standard external library system")
else()
  # If the pre-built library doesn't exist, we'll still continue since it will be
  # built from source as part of the build process
  message(STATUS "No pre-built FreeSASA library found. Will build from source.")
endif()

# Set compilation flags for FreeSASA if needed
if(APPLE)
  add_definitions(-DFREESASA_APPLE_PLATFORM)
ELSEIF(UNIX AND NOT APPLE)
  add_definitions(-DFREESASA_LINUX_PLATFORM)
ELSEIF(WIN32)
  add_definitions(-DFREESASA_WINDOWS_PLATFORM)
ENDIF()

message(STATUS "FreeSASA module configured for CMake")