#
#  Makefile for Osmium command line tool documentation.
#
#  You need 'pandoc' for this to work.
#

OSMIUM_VERSION := 0.0.1

PANDOC := pandoc

MARKDOWN_FILES := $(wildcard *.md)
MAN_PAGES_1 := osmium.1 osmium-apply-changes.1 osmium-cat.1 osmium-fileinfo.1 osmium-merge-changes.1 osmium-time-filter.1
MAN_PAGES_5 := osmium-file-formats.5

PUBDATE := `date +%Y-%m-%d`
PANDOC_MAN_OPTIONS := -s -t man --template manpage.template --variable "description=osmium/$(OSMIUM_VERSION)" --variable "date=$(PUBDATE)"
PANDOC_HTML_OPTIONS := -s -t html

all: $(MAN_PAGES_1) $(MAN_PAGES_5)

%.1: %.md
	$(PANDOC) $(PANDOC_MAN_OPTIONS) -o $@ $<

%.5: %.md
	$(PANDOC) $(PANDOC_MAN_OPTIONS) -o $@ $<

%.html: %.md
	$(PANDOC) $(PANDOC_HTML_OPTIONS) -o $@ $<

clean:
	rm -f *.1 *.5 *.html

