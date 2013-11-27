#
#  Makefile for Osmium command line tool documentation.
#
#  You need 'pandoc' for this to work.
#

OSMIUM_VERSION := 0.0.1

PANDOC := pandoc

MARKDOWN_FILES := $(wildcard *.md)
MAN_PAGES := $(MARKDOWN_FILES:.md=.1)

PUBDATE := `date +%Y-%m-%d`

all: $(MAN_PAGES)

%.1: %.md
	$(PANDOC) -s -t man --variable "description=osmium/$(OSMIUM_VERSION)" --variable "date=$(PUBDATE)" -o $@ $<

%.html: %.md
	$(PANDOC) -s -t html -o $@ $<

clean:
	rm -f *.1 *.html

