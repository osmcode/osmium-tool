#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - help
#
#-----------------------------------------------------------------------------

do_test(help1 "osmium"        "^Usage: .*Commands are:")
do_test(help2 "osmium help"   "^Usage: .*Commands are:")
do_test(help3 "osmium --help" "^Usage: .*Commands are:")
do_test(help4 "osmium -h"     "^Usage: .*Commands are:")

if(PANDOC)
    do_test(help_cat "osmium help cat" "^OSMIUM-CAT\\(1\\)")
endif()

do_test(help_topic_unknown "osmium help x" "^Unknown help topic 'x'.\n")


#-----------------------------------------------------------------------------
