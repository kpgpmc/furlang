#include "furas/gen.hpp"
#include "furas/lexer.hpp"

#include <fstream>
#include <furvm/module.hpp>
#include <iostream>

int main(int argc, char** argv) { // NOLINT
    if (argc < 2) {
        std::cerr << "feed me more arguments daddy >_<\n";
        return 1;
    }
    if (argc > 2) {
        std::cerr << "too much O_O\n";
        return 1;
    }

    const char* filepath = argv[1];

    std::ifstream file(filepath, std::ios::binary | std::ios::ate | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "file won't open >~<\n";
        return 1;
    }
    std::string content;
    content.resize(file.tellg());
    file.seekg(0);
    file.read(content.data(), static_cast<std::streamsize>(content.size()));

    auto result = furas::generator::generate(furas::lexer(filepath, content));
    if (result.error.type != furas::generator_error::Success) {
        std::cerr << result.error.message << '\n';
        return 1;
    }

    result.mod.serialize(std::cout);

    return 0;
}
