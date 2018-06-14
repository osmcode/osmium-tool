#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - help
#
#-----------------------------------------------------------------------------

do_test(help1 "osmium"        "^Usage: .*COMMANDS:")
do_test(help2 "osmium help"   "^Usage: .*COMMANDS:")
do_test(help3 "osmium --help" "^Usage: .*COMMANDS:")
do_test(help4 "osmium -h"     "^Usage: .*COMMANDS:")

do_test(help_topic_unknown "osmium help x" "^Unknown help topic 'x'.\n")


#-----------------------------------------------------------------------------
