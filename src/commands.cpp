
#include "cmd.hpp"

#include "command_add_locations_to_ways.hpp"
#include "command_apply_changes.hpp"
#include "command_cat.hpp"
#include "command_changeset_filter.hpp"
#include "command_check_refs.hpp"
#include "command_create_locations_index.hpp"
#include "command_derive_changes.hpp"
#include "command_diff.hpp"
#include "command_export.hpp"
#include "command_extract.hpp"
#include "command_fileinfo.hpp"
#include "command_getid.hpp"
#include "command_getparents.hpp"
#include "command_help.hpp"
#include "command_merge.hpp"
#include "command_merge_changes.hpp"
#include "command_query_locations_index.hpp"
#include "command_removeid.hpp"
#include "command_renumber.hpp"
#include "command_show.hpp"
#include "command_sort.hpp"
#include "command_tags_count.hpp"
#include "command_tags_filter.hpp"
#include "command_time_filter.hpp"

void register_commands(CommandFactory& cmd_factory) {
    cmd_factory.register_command("add-locations-to-ways", "Add node locations to ways", [&]() {
        return std::make_unique<CommandAddLocationsToWays>(cmd_factory);
    });

    cmd_factory.register_command("apply-changes", "Apply OSM change files to OSM data file", [&]() {
        return std::make_unique<CommandApplyChanges>(cmd_factory);
    });

    cmd_factory.register_command("cat", "Concatenate OSM files and convert to different formats", [&]() {
        return std::make_unique<CommandCat>(cmd_factory);
    });

    cmd_factory.register_command("changeset-filter", "Filter OSM changesets by different criteria", [&]() {
        return std::make_unique<CommandChangesetFilter>(cmd_factory);
    });

    cmd_factory.register_command("check-refs", "Check referential integrity of an OSM file", [&]() {
        return std::make_unique<CommandCheckRefs>(cmd_factory);
    });

    cmd_factory.register_command("create-locations-index", "Create node locations index on disk", [&]() {
        return std::make_unique<CommandCreateLocationsIndex>(cmd_factory);
    });

    cmd_factory.register_command("derive-changes", "Create OSM change files from two OSM data files", [&]() {
        return std::make_unique<CommandDeriveChanges>(cmd_factory);
    });

    cmd_factory.register_command("diff", "Display differences between OSM files", [&]() {
        return std::make_unique<CommandDiff>(cmd_factory);
    });

    cmd_factory.register_command("export", "Export OSM data", [&]() {
        return std::make_unique<CommandExport>(cmd_factory);
    });

    cmd_factory.register_command("extract", "Create geographic extract", [&]() {
        return std::make_unique<CommandExtract>(cmd_factory);
    });

    cmd_factory.register_command("fileinfo", "Show information about OSM file", [&]() {
        return std::make_unique<CommandFileinfo>(cmd_factory);
    });

    cmd_factory.register_command("getid", "Get objects with given ID from OSM file", [&]() {
        return std::make_unique<CommandGetId>(cmd_factory);
    });

    cmd_factory.register_command("getparents", "Get parents of objects from OSM file", [&]() {
        return std::make_unique<CommandGetParents>(cmd_factory);
    });

    cmd_factory.register_command("help", "Show osmium help", [&]() {
        return std::make_unique<CommandHelp>(cmd_factory);
    });

    cmd_factory.register_command("merge-changes", "Merge several OSM change files into one", [&]() {
        return std::make_unique<CommandMergeChanges>(cmd_factory);
    });
    cmd_factory.register_command("merge", "Merge several sorted OSM files into one", [&]() {
        return std::make_unique<CommandMerge>(cmd_factory);
    });

    cmd_factory.register_command("query-locations-index", "Query node locations index on disk", [&]() {
        return std::make_unique<CommandQueryLocationsIndex>(cmd_factory);
    });

    cmd_factory.register_command("removeid", "Remove objects from OSM file by ID", [&]() {
        return std::make_unique<CommandRemoveId>(cmd_factory);
    });

    cmd_factory.register_command("renumber", "Renumber IDs in OSM file", [&]() {
        return std::make_unique<CommandRenumber>(cmd_factory);
    });

    cmd_factory.register_command("show", "Show OSM file contents", [&]() {
        return std::make_unique<CommandShow>(cmd_factory);
    });

    cmd_factory.register_command("sort", "Sort OSM data files", [&]() {
        return std::make_unique<CommandSort>(cmd_factory);
    });

    cmd_factory.register_command("tags-count", "Count OSM tags", [&]() {
        return std::make_unique<CommandTagsCount>(cmd_factory);
    });

    cmd_factory.register_command("tags-filter", "Filter OSM data based on tags", [&]() {
        return std::make_unique<CommandTagsFilter>(cmd_factory);
    });

    cmd_factory.register_command("time-filter", "Filter OSM data from a point in time or a time span out of a history file", [&]() {
        return std::make_unique<CommandTimeFilter>(cmd_factory);
    });
}

