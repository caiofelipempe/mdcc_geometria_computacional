// error.hpp
#pragma once
#include <type_traits>
#ifndef NDEBUG
#include <string>
#endif

template <typename T>
concept EnumType = std::is_enum_v<T>;

template <EnumType Code>
struct Error {
    Code code;
#ifndef NDEBUG
    std::string message;
#else
    char _empty{};
#endif

    static Error make(Code code, [[maybe_unused]] std::string msg = {}) {
#ifndef NDEBUG
        return {code, std::move(msg)};
#else
        return {code};
#endif
    }
};
