#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - help
#
#-----------------------------------------------------------------------------

do_test(version  "osmium --version" "^osmium version ${OSMIUM_VERSION}\n")
do_test(help     "osmium help"      "^Usage: .*Commands are:")
do_test(help2    "osmium --help"    "^Usage: .*Commands are:")

if(PANDOC)
    do_test(help_cat "osmium help cat" "^OSMIUM-CAT\\(1\\)")
endif()


#-----------------------------------------------------------------------------
