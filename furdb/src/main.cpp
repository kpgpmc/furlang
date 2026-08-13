#include "command.hpp"
#include "context.hpp"
#include "furvm/function.hpp"

#include <fstream>
#include <furvm/fwd.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

// taken from furvm uwu :3 ^^ nya~ ngh~
static void print_thing(const furvm::thing<furvm::thing_allocator>& thing) {
    using namespace furvm;

    switch (thing.true_type().type) {
    case thing_type::S8: std::cout << thing.cast_to<thing_type::s16>(); break;
    case thing_type::S16: std::cout << thing.get<thing_type::s16>(); break;
    case thing_type::S32: std::cout << thing.get<thing_type::s32>(); break;
    case thing_type::S64: std::cout << thing.get<thing_type::s64>(); break;
    case thing_type::U8: std::cout << thing.get<thing_type::u8>(); break;
    case thing_type::U16: std::cout << thing.get<thing_type::u16>(); break;
    case thing_type::U32: std::cout << thing.get<thing_type::u32>(); break;
    case thing_type::U64: std::cout << thing.get<thing_type::u64>(); break;
    case thing_type::Array:
        std::cout << "{ ";
        for (thing_type::u64 i = 0; i < thing.length(); ++i) {
            if (i > 0) std::cout << ", ";
            print_thing(thing.at(i));
        }
        std::cout << " }";
        break;
    default: std::cerr << "{Type not recognized}";
    }
}

// TODO: Argument parsing
// TODO: Multiple modules
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <module.fmod>\n";
        return 1;
    }

    static std::unordered_map<std::string_view, command*> s_commands;
    s_commands["quit"] = s_commands["q"] = new quit_command();
    s_commands["run"] = s_commands["r"] = new run_command();

    try {
        std::ifstream file(argv[1], std::ios::binary | std::ios::in);

        context ctx;
        try {
            ctx.mod = ctx.context->emplace(argv[1], furvm::mod::load(file));
        } catch (std::exception ex) {
            std::cerr << "Failed to load module " << argv[1] << ": " << ex.what() << '\n';
            return 1;
        }
        ctx.executor     = &ctx.context->allocate_executor();
        ctx.mainFunction = ctx.mod->function_at("main", furvm::function_sig{});

        ctx.mod->set_native_function("println", [](furvm::executor& executor) {
            auto arg = executor.load_thing(0);
            print_thing(*arg);
            std::cout << '\n';
        });

        std::string inputLine;
        std::string prevInput;
        while (ctx.running) {
            std::swap(inputLine, prevInput);
            std::cout << "(furdb) ";
            if (!std::getline(std::cin, inputLine)) break;

            if (inputLine.empty()) {
                inputLine = prevInput;
            }

            auto info = command::parse(inputLine);
            auto it   = s_commands.find(info.commandName);
            if (it == s_commands.end()) {
                std::cerr << "Unknown command \"" << info.commandName << "\"\n";
                continue;
            }
            it->second->execute(ctx, info);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Uncaught exception: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
