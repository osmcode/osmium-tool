#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - misc
#
#-----------------------------------------------------------------------------

do_test(version1 "osmium --version" "^osmium version ${PROJECT_VERSION}")
do_test(version2 "osmium version"   "^osmium version ${PROJECT_VERSION}")

do_test(unknown_command "osmium x" "^Unknown command or option 'x'. Try 'osmium help'.\n")


#-----------------------------------------------------------------------------
