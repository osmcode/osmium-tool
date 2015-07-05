#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - misc
#
#-----------------------------------------------------------------------------

do_test(version1 "osmium --version" "^osmium version ${OSMIUM_VERSION}\n")
do_test(version2 "osmium version"   "^osmium version ${OSMIUM_VERSION}\n")

do_test(unknown_command "osmium x" "^Unknown command 'x'. Try 'osmium help'.\n")


#-----------------------------------------------------------------------------
