#include "command.hpp"

#include "furvm/executor.hpp"
#include "furvm/function.hpp"
#include "furvm/instruction.hpp"
#include "furvm/module.hpp"
#include "furvm/thing.hpp"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <utility>

command_info command::parse(std::string_view line) {
    command_info info;

    while (!line.empty() && std::isspace(line.front()) != 0)
        line = line.substr(1);
    std::size_t length = 0;
    while (line.size() > length && std::isalnum(line.at(length)) != 0)
        ++length;
    info.commandName = line.substr(0, length);
    line             = line.substr(length);

    while (true) {
        while (!line.empty() && std::isspace(line.front()) != 0)
            line = line.substr(1);
        if (line.empty()) break;
        length = 0;
        while (line.size() > length && std::isalnum(line.at(length)) != 0)
            ++length;
        info.args.push_back(line.substr(0, length));
        line = line.substr(length);
    }

    return std::move(info);
}

void quit_command::execute(context& ctx, const command_info& info) {
    static std::random_device              s_rd;
    static std::mt19937                    s_gen(s_rd());
    static std::uniform_int_distribution<> s_dis(1, 1000);

    int roll = s_dis(s_gen);

    std::cout << "Farewell";
    if (roll == 1) {
        std::cout << ", stasiu pensive";
    } else if (roll <= 100) {
        std::cout << ", kromek-man";
    }
    std::cout << "!\n";

    ctx.running = false;
}

void run_command::execute(context& ctx, const command_info& info) {
    if (!ctx.executor->done()) {
        std::cout << "A program is currently running, do you want to kill it? (y/N) ";
        std::string answer;
        if (!std::getline(std::cin, answer)) {
            ctx.running = false;
            return;
        }
        if (answer != "y" && answer != "Y" && answer != "yes") return;
        ctx.kill();
    }
    std::cout << "Running the program\n";
    ctx.run();
}

static void breakpoint_hit(furvm::executor& executor, void* data) {
    std::cout << "Breakpoint hit\n";
    context* ctx = reinterpret_cast<context*>(data);
    ctx->halt    = true;

    ctx->print_instruction();
}

void break_command::execute(context& ctx, const command_info& info) {
    if (info.args.empty()) {
        std::cerr << "Usage: " << info.commandName << " <function name>\n";
        return;
    }

    std::size_t count = 0;
    for (const auto& [id, sigPair] : ctx.mod->function_map()) {
        if (sigPair.first != info.args[0]) continue;
        const auto& func = ctx.mod->function_at(id);
        if (func->type() != furvm::function_t::Normal) continue;
        count += 1;
        ctx.mod->set_breakpoint(func->position(), furvm::breakpoint{ breakpoint_hit, &ctx });
    }
    if (count == 0) {
        std::cerr << "No function \"" << info.args[0] << "\" found!\n";
        return;
    }
    std::cout << "Breakpoint set in " << count << " places\n";
}
