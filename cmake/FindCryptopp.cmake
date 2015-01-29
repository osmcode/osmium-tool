# - Find cryptopp
# Find the native CRYPTOPP headers and libraries.
#
#  CRYPTOPP_INCLUDE_DIRS - where to find include files
#  CRYPTOPP_LIBRARIES    - List of libraries when using cryptopp.
#  CRYPTOPP_FOUND        - True if cryptopp found.

# The library is called 'crypto++' on Linux, on OS/X it is called 'cryptopp'.

# Look for the header file.
find_path(CRYPTOPP_INCLUDE_DIR NAMES cryptopp/sha.h)

# Look for the library.
find_library(CRYPTOPP_LIBRARY NAMES cryptopp libcryptopp crypto++ libcrypto++)

# handle the QUIETLY and REQUIRED arguments and set CRYPTOPP_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CRYPTOPP
                                  REQUIRED_VARS CRYPTOPP_LIBRARY CRYPTOPP_INCLUDE_DIR)

# Copy the results to the output variables.
if(CRYPTOPP_FOUND)
  set(CRYPTOPP_INCLUDE_DIRS ${CRYPTOPP_INCLUDE_DIR})
  set(CRYPTOPP_LIBRARIES ${CRYPTOPP_LIBRARY})
  add_definitions(-DOSMIUM_WITH_CRYPTOPP)
endif()

