# FreeSASA external library configuration for CMake

# Add FreeSASA include directory
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/include)
include_directories(${CMAKE_SOURCE_DIR}/../external/freesasa/src)

# Define the path to the static library
set(FREESASA_LIB_PATH ${CMAKE_SOURCE_DIR}/../../external/freesasa/lib/libfreesasa.a)

if(EXISTS ${FREESASA_LIB_PATH})
  # Use the pre-built static library directly
  message(STATUS "Using pre-built FreeSASA library at ${FREESASA_LIB_PATH}")
  
  # Add the full path to LINK_EXTERNAL_LIBS
  set(LINK_EXTERNAL_LIBS ${LINK_EXTERNAL_LIBS} ${FREESASA_LIB_PATH})
  message(STATUS "Added FreeSASA library path directly to LINK_EXTERNAL_LIBS")
else()
  # If the pre-built library doesn't exist, we should raise an error
  message(FATAL_ERROR "Pre-built FreeSASA library not found at ${FREESASA_LIB_PATH}. Please make sure it exists.")
endif()

message(STATUS "FreeSASA library configured for CMake")