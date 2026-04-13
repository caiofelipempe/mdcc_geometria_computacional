#pragma once

#ifndef NDEBUG
#include <string>
#endif

struct Error {
#ifndef NDEBUG
    const std::string message;
#endif

    static Error make([[maybe_unused]] std::string msg) {
#ifndef NDEBUG
        return {std::move(msg)};
#else
        return {};
#endif
    }
};