#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "context.hpp"

#include <string_view>
#include <vector>

struct command_info {
    std::string_view              commandName;
    std::vector<std::string_view> args;
};

class command {
public:
    command()          = default;
    virtual ~command() = default;

    command(command&&) noexcept            = default;
    command& operator=(command&&) noexcept = default;
    command(const command&)                = delete;
    command& operator=(const command&)     = delete;
public:
    virtual void execute(context& ctx, const command_info& info) = 0;
public:
    static command_info parse(std::string_view line);
};

class quit_command final : public command {
public:
    void execute(context& ctx, const command_info& info) override;
};

class run_command final : public command {
public:
    void execute(context& ctx, const command_info& info) override;
};

class break_command final : public command {
public:
    void execute(context& ctx, const command_info& info) override;
};

#endif // COMMAND_HPP
