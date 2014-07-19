#------------------------------------------------------------------------------
#
#  Makefile for Osmium Command Line Tool
#
#------------------------------------------------------------------------------
#
#  You can set several environment variables before running make if you don't
#  like the defaults:
#
#  CXX                - Your C++ compiler.
#  CPLUS_INCLUDE_PATH - Include file search path.
#  CXXFLAGS           - Extra compiler flags.
#  LDFLAGS            - Extra linker flags.
#  
#------------------------------------------------------------------------------

CXXFLAGS += -O3
#CXXFLAGS += -g
CXXFLAGS += -std=c++11 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../libosmium/include

PREFIX ?= /usr

OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

CXXFLAGS_WARNINGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast

LIB_PBF    := -pthread -lz -lprotobuf-lite -losmpbf
LIB_EXPAT  := -lexpat
LIB_GZIP   := -lz
LIB_BZIP2  := -lbz2

LIB_PRGOPT := -lboost_program_options
LIB_CRYPTO := -lcrypto++

PROGRAMS := osmium
COMMANDS_H := $(wildcard src/command_*.hpp)
COMMANDS_O := $(COMMANDS_H:.hpp=.o)

INCLUDES := src/osmc.hpp

# We use the numeric id 0 here because different systems (Linux vs. BSD)
# use different names for the "root group".
INSTALL_GROUP := 0

CPPCHECK_OPTIONS := --enable=warning,style,performance,portability,information,missingInclude

# cpp doesn't find system includes for some reason, suppress that report
CPPCHECK_OPTIONS += --suppress=missingIncludeSystem

# temp fix for http://sourceforge.net/apps/trac/cppcheck/ticket/4966
CPPCHECK_OPTIONS += --suppress=constStatement

.PHONY: all check clean doc

all: $(PROGRAMS)

src/main.o: src/main.cpp $(COMMANDS_H) $(INCLUDES)
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $<

src/command_%.o: src/command_%.cpp src/command_%.hpp $(INCLUDES)
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $<

osmium: src/main.o $(COMMANDS_O)
	$(CXX) -o $@ $< $(COMMANDS_O) $(LDFLAGS) $(LIB_PBF) $(LIB_EXPAT) $(LIB_GZIP) $(LIB_BZIP2) $(LIB_PRGOPT) $(LIB_CRYPTO)

cppcheck:
	cppcheck --std=c++11 $(CPPCHECK_OPTIONS) src/*pp

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none --recursive src/\*pp

clean:
	rm -f src/*.o core $(PROGRAMS)
	$(MAKE) -C doc clean

doc:
	$(MAKE) -C doc

install:
	install -m 755 -g $(INSTALL_GROUP) -o root -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 -g $(INSTALL_GROUP) -o root -d $(DESTDIR)$(PREFIX)/share/doc/osmium
	install -m 755 -g $(INSTALL_GROUP) -o root osmium $(DESTDIR)$(PREFIX)/bin/osmium
	install -m 644 -g $(INSTALL_GROUP) -o root README.md $(DESTDIR)$(PREFIX)/share/doc/osmium/README.md

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

