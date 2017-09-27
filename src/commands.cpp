
#include "cmd.hpp"

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
#include "command_values_filter.hpp"
#include "command_time_filter.hpp"

void register_commands(CommandFactory& cmd_factory) {
    cmd_factory.register_command("add-locations-to-ways", "Add node locations to ways", [&]() {
        return new CommandAddLocationsToWays{cmd_factory};
    });

    cmd_factory.register_command("apply-changes", "Apply OSM change files to OSM data file", [&]() {
        return new CommandApplyChanges{cmd_factory};
    });

    cmd_factory.register_command("cat", "Concatenate OSM files and convert to different formats", [&]() {
        return new CommandCat{cmd_factory};
    });

    cmd_factory.register_command("changeset-filter", "Filter OSM changesets by different criteria", [&]() {
        return new CommandChangesetFilter{cmd_factory};
    });

    cmd_factory.register_command("check-refs", "Check referential integrity of an OSM file", [&]() {
        return new CommandCheckRefs{cmd_factory};
    });

    cmd_factory.register_command("derive-changes", "Create OSM change files from two OSM data files", [&]() {
        return new CommandDeriveChanges{cmd_factory};
    });

    cmd_factory.register_command("diff", "Display differences between OSM files", [&]() {
        return new CommandDiff{cmd_factory};
    });

    cmd_factory.register_command("export", "Export OSM data", [&]() {
        return new CommandExport{cmd_factory};
    });

    cmd_factory.register_command("extract", "Create geographic extract", [&]() {
        return new CommandExtract{cmd_factory};
    });

    cmd_factory.register_command("fileinfo", "Show information about OSM file", [&]() {
        return new CommandFileinfo{cmd_factory};
    });

    cmd_factory.register_command("getid", "Get objects with given ID from OSM file", [&]() {
        return new CommandGetId{cmd_factory};
    });

    cmd_factory.register_command("help", "Show osmium help", [&]() {
        return new CommandHelp{cmd_factory};
    });

    cmd_factory.register_command("merge-changes", "Merge several OSM change files into one", [&]() {
        return new CommandMergeChanges{cmd_factory};
    });
    cmd_factory.register_command("merge", "Merge several sorted OSM files into one", [&]() {
        return new CommandMerge{cmd_factory};
    });

    cmd_factory.register_command("renumber", "Renumber IDs in OSM file", [&]() {
        return new CommandRenumber{cmd_factory};
    });

    cmd_factory.register_command("show", "Show OSM file contents", [&]() {
        return new CommandShow{cmd_factory};
    });

    cmd_factory.register_command("sort", "Sort OSM data files", [&]() {
        return new CommandSort{cmd_factory};
    });

    cmd_factory.register_command("tags-filter", "Filter OSM data based on tags", [&]() {
        return new CommandTagsFilter{cmd_factory};
    });

    cmd_factory.register_command("values-filter", "Filter OSM data based on tag values", [&]() {
        return new CommandValuesFilter{cmd_factory};
    });

    cmd_factory.register_command("time-filter", "Filter OSM data from a point in time or a time span out of a history file", [&]() {
        return new CommandTimeFilter{cmd_factory};
    });
}

