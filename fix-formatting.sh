#!/bin/sh
#
#  fix-formatting
#

exec astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none src/*pp

