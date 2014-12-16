# - Find Osmium
# Find the Osmium headers.
#
#  OSMIUM_INCLUDE_DIRS - Where to find include files.
#  OSMIUM_FOUND        - True if Osmium found.

# Look for the header file.
find_path(OSMIUM_INCLUDE_DIR osmium/osm.hpp
    PATH_SUFFIXES include
    PATHS
    ../libosmium
    ../../libosmium
    libosmium
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr/
    /opt/local # DarwinPorts
    /opt
)

# handle the QUIETLY and REQUIRED arguments and set OSMIUM_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSMIUM REQUIRED_VARS OSMIUM_INCLUDE_DIR)

# Copy the results to the output variables.
if(OSMIUM_FOUND)
  set(OSMIUM_INCLUDE_DIRS ${OSMIUM_INCLUDE_DIR})
endif()

if(Osmium_FIND_REQUIRED AND NOT OSMIUM_FOUND)
  message(FATAL_ERROR "Can not find libosmium headers, please install them or configure the paths")
endif()


if(";${Osmium_FIND_COMPONENTS};" MATCHES ";io;")
  find_package(OSMPBF)
  find_package(Protobuf)
  find_package(ZLIB)
  find_package(BZip2)
  find_package(EXPAT)
  find_package(Threads)

  if(OSMPBF_FOUND AND PROTOBUF_FOUND AND ZLIB_FOUND AND BZIP2_FOUND AND EXPAT_FOUND AND Threads_FOUND)
    list(APPEND OSMIUM_LIBRARIES
      ${OSMPBF_LIBRARIES} ${PROTOBUF_LITE_LIBRARY}
      ${ZLIB_LIBRARY} ${BZIP2_LIBRARIES}
      ${EXPAT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    list(APPEND OSMIUM_INCLUDE_DIRS
      ${OSMPBF_INCLUDE_DIRS}
      ${PROTOBUF_INCLUDE_DIR}
      ${EXPAT_INCLUDE_DIR}
      ${BZIP2_INCLUDE_DIR}
      ${ZLIB_INCLUDE_DIR}
    )
    if(WIN32)
      list(APPEND OSMIUM_LIBRARIES ws2_32)
    endif()
  else()
    set(MISSING_LIBRARIES 1)
    if(Osmium_FIND_REQUIRED)
      message(FATAL_ERROR "Can not find some libraries for Osmium input/output formats, please install them or configure the paths")
    endif()
  endif()
endif()

if(";${Osmium_FIND_COMPONENTS};" MATCHES ";geos;")
  ##### Find GEOS Library
  find_path(GEOS_INCLUDE_DIR geos/geom.h)
  find_library(GEOS_LIBRARY NAMES geos)
  if(GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
    message(STATUS "Found GEOS: " ${GEOS_LIBRARY})
    SET(GEOS_FOUND 1)
    list(APPEND OSMIUM_LIBRARIES ${GEOS_LIBRARY})
    list(APPEND OSMIUM_INCLUDE_DIRS ${GEOS_INCLUDE_DIR})
  else()
    set(MISSING_LIBRARIES 1)
    if(Osmium_FIND_REQUIRED)
      message(FATAL_ERROR "GEOS library is required but not found, please install it or configure the paths")
    endif()
  endif()
endif()

if(";${Osmium_FIND_COMPONENTS};" MATCHES ";gdal;")
  find_package(GDAL)
  if(NOT GDAL_FOUND)
    set(MISSING_LIBRARIES 1)
    if(Osmium_FIND_REQUIRED)
      message(FATAL_ERROR "GDAL library is required but not found, please install it or configure the paths")
    endif()
  else()
    list(APPEND OSMIUM_LIBRARIES ${GDAL_LIBRARY})
    list(APPEND OSMIUM_INCLUDE_DIRS ${GDAL_INCLUDE_DIR})
  endif()
endif()

if(";${Osmium_FIND_COMPONENTS};" MATCHES ";proj;")
  ##### Find Proj.4 Library
  find_path(PROJ_INCLUDE_DIR proj_api.h)
  find_library(PROJ_LIBRARY NAMES proj)
  if(PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
    message(STATUS "Found PROJ: " ${PROJ_LIBRARY})
    set(PROJ_FOUND 1)
    list(APPEND OSMIUM_LIBRARIES ${PROJ_LIBRARY})
    list(APPEND OSMIUM_INCLUDE_DIRS ${PROJ_INCLUDE_DIR})
  else()
    set(MISSING_LIBRARIES 1)
    if(Osmium_FIND_REQUIRED)
      message(FATAL_ERROR "PROJ library is required but not found, please install it or configure the paths")
    endif()
  endif()
endif()

if(";${Osmium_FIND_COMPONENTS};" MATCHES ";sparsehash;")
  ##### Find Google SparseHash
  find_path(SPARSEHASH_INCLUDE_DIR google/sparsetable)
  if(SPARSEHASH_INCLUDE_DIR)
    message(STATUS "Found SparseHash")
    set(SPARSEHASH_FOUND 1)
    list(APPEND OSMIUM_INCLUDE_DIRS ${SPARSEHASH_INCLUDE_DIR})
  else()
    set(MISSING_LIBRARIES 1)
    if(Osmium_FIND_REQUIRED)
      message(FATAL_ERROR "SparseHash library is required but not found, please install it or configure the paths")
    endif()
  endif()
endif()

list(REMOVE_DUPLICATES OSMIUM_INCLUDE_DIRS)
list(REMOVE_DUPLICATES OSMIUM_LIBRARIES)

if(Osmium_DEBUG)
  message(STATUS "OSMIUM_LIBRARIES=" ${OSMIUM_LIBRARIES})
  message(STATUS "OSMIUM_INCLUDE_DIRS=" ${OSMIUM_INCLUDE_DIRS})
endif()
