#include "command.hpp"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <random>
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
    ctx.run();
    std::cout << "Execution finished\n";
}
