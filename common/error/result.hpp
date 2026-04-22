// result.hpp
#pragma once
#include <expected>

template <typename T, typename E, bool Safe = true>
using Result =
    std::conditional_t<
        Safe,
        std::expected<T, E>,
        T
    >;