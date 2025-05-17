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