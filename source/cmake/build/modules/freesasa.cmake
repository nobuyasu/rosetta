# FreeSASA external library configuration for CMake

# Add FreeSASA include directories
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)

# Use the pre-built static library
set(FREESASA_LIB_PATH ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a)

if(EXISTS ${FREESASA_LIB_PATH})
  # Use the pre-built static library directly
  message(STATUS "Found pre-built FreeSASA library at ${FREESASA_LIB_PATH}")
  
  # Add the library directly to LINK_EXTERNAL_LIBS to ensure it's linked with applications
  set(LINK_EXTERNAL_LIBS ${LINK_EXTERNAL_LIBS} ${FREESASA_LIB_PATH})
  message(STATUS "Added pre-built FreeSASA library to LINK_EXTERNAL_LIBS")
else()
  message(FATAL_ERROR "Pre-built FreeSASA library not found at ${FREESASA_LIB_PATH}. Please build or obtain the library first.")
endif()

message(STATUS "FreeSASA module configured for CMake using pre-built library")