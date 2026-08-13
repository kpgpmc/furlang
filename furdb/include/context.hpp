#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <furvm/context.hpp>
#include <furvm/executor.hpp>
#include <furvm/fwd.hpp>
#include <furvm/module.hpp>

struct context {
    furvm::context_p  context = std::make_shared<furvm::context>();
    furvm::mod_h      mod;
    furvm::executor*  executor = nullptr;
    furvm::function_h mainFunction;

    bool running = true;

    void run();
};

#endif // CONTEXT_HPP
