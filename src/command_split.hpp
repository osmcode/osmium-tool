#ifndef COMMAND_SPLIT_HPP
#define COMMAND_SPLIT_HPP

#include "cmd.hpp"

#include <osmium/io/file.hpp>
#include <string>
#include <vector>

class CommandSplit : public CommandWithSingleOSMInput {
    std::string m_output_prefix = "split";
    std::string m_output_format = "osm.pbf";
    bool m_overwrite = false;
    bool m_use_fsync = false;
    bool m_split_details = false;
    bool m_skip_empty_tags = false;

public:
    explicit CommandSplit(const CommandFactory& command_factory);

    bool setup(const std::vector<std::string>& arguments) override;

    void show_arguments() override;

    bool run() override;

    const char* name() const noexcept override {
        return "split";
    }

    const char* synopsis() const noexcept override {
        return "osmium split [OPTIONS] OSM-FILE";
    }
};

#endif // COMMAND_SPLIT_HPP
