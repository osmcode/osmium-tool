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

