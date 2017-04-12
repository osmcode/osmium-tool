
#include "command_add_locations_to_ways.hpp"
#include "command_apply_changes.hpp"
#include "command_cat.hpp"
#include "command_changeset_filter.hpp"
#include "command_check_refs.hpp"
#include "command_derive_changes.hpp"
#include "command_diff.hpp"
#include "command_export.hpp"
#include "command_extract.hpp"
#include "command_fileinfo.hpp"
#include "command_getid.hpp"
#include "command_help.hpp"
#include "command_merge_changes.hpp"
#include "command_merge.hpp"
#include "command_renumber.hpp"
#include "command_show.hpp"
#include "command_sort.hpp"
#include "command_tags_filter.hpp"
#include "command_time_filter.hpp"

void register_commands() {
    CommandFactory::add("add-locations-to-ways", "Add node locations to ways", []() {
        return new CommandAddLocationsToWays();
    });

    CommandFactory::add("apply-changes", "Apply OSM change files to OSM data file", []() {
        return new CommandApplyChanges();
    });

    CommandFactory::add("cat", "Concatenate OSM files and convert to different formats", []() {
        return new CommandCat();
    });

    CommandFactory::add("changeset-filter", "Filter OSM changesets by different criteria", []() {
        return new CommandChangesetFilter();
    });

    CommandFactory::add("check-refs", "Check referential integrity of an OSM file", []() {
        return new CommandCheckRefs();
    });

    CommandFactory::add("derive-changes", "Create OSM change files from two OSM data files", []() {
        return new CommandDeriveChanges();
    });

    CommandFactory::add("diff", "Display differences between OSM files", []() {
        return new CommandDiff();
    });

    CommandFactory::add("export", "Export OSM data", []() {
        return new CommandExport();
    });

    CommandFactory::add("extract", "Create geographic extract", []() {
        return new CommandExtract();
    });

    CommandFactory::add("fileinfo", "Show information about OSM file", []() {
        return new CommandFileinfo();
    });

    CommandFactory::add("getid", "Get objects with given ID from OSM file", []() {
        return new CommandGetId();
    });

    CommandFactory::add("help", "Show osmium help", []() {
        return new CommandHelp();
    });

    CommandFactory::add("merge-changes", "Merge several OSM change files into one", []() {
        return new CommandMergeChanges();
    });
    CommandFactory::add("merge", "Merge several sorted OSM files into one", []() {
        return new CommandMerge();
    });

    CommandFactory::add("renumber", "Renumber IDs in OSM file", []() {
        return new CommandRenumber();
    });

    CommandFactory::add("show", "Show OSM file contents", []() {
        return new CommandShow();
    });

    CommandFactory::add("sort", "Sort OSM data files", []() {
        return new CommandSort();
    });

    CommandFactory::add("tags-filter", "Filter OSM data based on tags", []() {
        return new CommandTagsFilter();
    });

    CommandFactory::add("time-filter", "Filter OSM data from a point in time or a time span out of a history file", []() {
        return new CommandTimeFilter();
    });
}

