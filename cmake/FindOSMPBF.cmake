# Locate OSMPBF library
# This module defines
#  OSMPBF_FOUND, if false, do not try to link to OSMPBF
#  OSMPBF_LIBRARIES
#  OSMPBF_INCLUDE_DIRS, where to find OSMPBF.hpp
#
# Note that the expected include convention is
#  #include <osmpbf/osmpbf.h>
# and not
#  #include <osmpbf.h>

FIND_PATH(OSMPBF_INCLUDE_DIR osmpbf/osmpbf.h
  HINTS
  $ENV{OSMPBF_DIR}
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /opt/local # DarwinPorts
  /opt
)

FIND_LIBRARY(OSMPBF_LIBRARY
  NAMES osmpbf
  HINTS
  $ENV{OSMPBF_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /opt/local
  /opt
)

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OSMPBF_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSMPBF DEFAULT_MSG OSMPBF_LIBRARY OSMPBF_INCLUDE_DIR)

# Copy the results to the output variables.
if(OSMPBF_FOUND)
  set(OSMPBF_INCLUDE_DIRS ${OSMPBF_INCLUDE_DIR})
  set(OSMPBF_LIBRARIES ${OSMPBF_LIBRARY})
endif()

