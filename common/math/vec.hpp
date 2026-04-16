#pragma once

#include "error.hpp"

#include <array>
#include <cmath>
#include <expected>
#include <stdexcept>
#include <type_traits>

namespace geometry {

// ------------------------------------------------------------
// Base aliases & helpers
// ------------------------------------------------------------

template <typename T, std::size_t N>
using Vec = std::array<T, N>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
constexpr T default_epsilon() noexcept {
    if constexpr (std::is_floating_point_v<T>)
        return static_cast<T>(1e-8);
    else
        return T{};
}

// ------------------------------------------------------------
// Arithmetic operators
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator+(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = a[i] + b[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator-(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = a[i] - b[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator-(const Vec<T, N>& v) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = -v[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator*(const Vec<T, N>& v, T s) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = v[i] * s;
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator*(T s, const Vec<T, N>& v) noexcept {
    return v * s;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator/(const Vec<T, N>& v, T s) {
    if constexpr (!std::is_floating_point_v<T>)
        if (s == T{}) throw std::domain_error("Division by zero");

    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = v[i] / s;
    return r;
}

// ------------------------------------------------------------
// Comparisons
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
constexpr bool operator==(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        if (a[i] != b[i]) return false;
    return true;
}

template <Arithmetic T, std::size_t N>
constexpr bool operator!=(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    return !(a == b);
}

// ------------------------------------------------------------
// Dot / length
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
constexpr T dot(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    T sum{};
    for (std::size_t i = 0; i < N; ++i)
        sum += a[i] * b[i];
    return sum;
}

template <Arithmetic T, std::size_t N>
constexpr T length_squared(const Vec<T, N>& v) noexcept {
    return dot(v, v);
}

template <Arithmetic T, std::size_t N>
T length(const Vec<T, N>& v) {
    if constexpr (std::is_floating_point_v<T>)
        return std::sqrt(length_squared(v));
    else
        return static_cast<T>(
            std::sqrt(static_cast<double>(length_squared(v)))
        );
}

// ------------------------------------------------------------
// Normalization
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalized(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));

    return v / std::sqrt(ls);
}

template <Arithmetic T, std::size_t N>
std::optional<Error>
normalize(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);
    if (ls <= eps * eps)
        return Error::make("Vector is zero");

    v /= std::sqrt(ls);
    return std::nullopt;
}

template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalizedIfGreaterThanOne(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);

    // vetor zero continua inválido
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));

    // |v| > 1 ?
    if (ls > T(1))
        return v / std::sqrt(ls);

    return v;
}

template <Arithmetic T, std::size_t N>
std::optional<Error>
normalizeIfGreaterThanOne(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);

    if (ls <= eps * eps)
        return Error::make("Vector is zero");

    if (ls > T(1))
        v /= std::sqrt(ls);

    return std::nullopt;
}

template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalizedIfSmallerThanOne(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);

    // vetor zero ainda é inválido
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));

    // |v| < 1 ?
    if (ls < T(1))
        return v / std::sqrt(ls);

    return v;
}

template <Arithmetic T, std::size_t N>
std::optional<Error>
normalizeIfSmallerThanOne(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    T ls = length_squared(v);

    if (ls <= eps * eps)
        return Error::make("Vector is zero");

    if (ls < T(1))
        v /= std::sqrt(ls);

    return std::nullopt;
}

// ------------------------------------------------------------
// Cross product
// ------------------------------------------------------------

template <Arithmetic T>
constexpr T cross(const Vec<T, 2>& a, const Vec<T, 2>& b) noexcept {
    return a[0] * b[1] - a[1] * b[0];
}

template <Arithmetic T>
constexpr Vec<T, 3> cross(const Vec<T, 3>& a, const Vec<T, 3>& b) noexcept {
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

template <Arithmetic T>
constexpr Vec<T, 4> cross(
    const Vec<T, 4>& a,
    const Vec<T, 4>& b,
    const Vec<T, 4>& c
) noexcept {
    return {
        +(a[1] * (b[2] * c[3] - b[3] * c[2])
        - a[2] * (b[1] * c[3] - b[3] * c[1])
        + a[3] * (b[1] * c[2] - b[2] * c[1])),

        -(a[0] * (b[2] * c[3] - b[3] * c[2])
        - a[2] * (b[0] * c[3] - b[3] * c[0])
        + a[3] * (b[0] * c[2] - b[2] * c[0])),

        +(a[0] * (b[1] * c[3] - b[3] * c[1])
        - a[1] * (b[0] * c[3] - b[3] * c[0])
        + a[3] * (b[0] * c[1] - b[1] * c[0])),

        -(a[0] * (b[1] * c[2] - b[2] * c[1])
        - a[1] * (b[0] * c[2] - b[2] * c[0])
        + a[2] * (b[0] * c[1] - b[1] * c[0]))
    };
}

// ------------------------------------------------------------
// Pseudoangle (2D)
// ------------------------------------------------------------

template <Arithmetic T>
std::expected<T, Error>
pseudoangle(const Vec<T, 2>& v, T eps = default_epsilon<T>()) {
    const T x = v[0];
    const T y = v[1];

    if (std::abs(x) <= eps && std::abs(y) <= eps)
        return std::unexpected(Error::make("Vector is zero"));

    const T ax = std::abs(x);
    const T ay = std::abs(y);

    if (ax >= ay) {
        T t = ay / ax;
        if (x >= 0)
            return (y >= 0) ? t : T(8) - t;
        else
            return (y >= 0) ? T(4) - t : T(4) + t;
    } else {
        T t = ax / ay;
        if (y >= 0)
            return (x >= 0) ? T(2) - t : T(2) + t;
        else
            return (x >= 0) ? T(6) + t : T(6) - t;
    }
}

template <Arithmetic T>
std::expected<T, Error>
pseudoangleAlt(const Vec<T, 2>& v, T eps = default_epsilon<T>()) {
    const T x = v[0];
    const T y = v[1];

    if (std::abs(x) <= eps && std::abs(y) <= eps)
        return std::unexpected(Error::make("Vector is zero"));

    const T ax = std::abs(x);
    const T ay = std::abs(y);

    if (x >= T{}) {
        return (y >= T{})
            ? y / (ax + ay)
            : T(3) + x / (ax + ay);
    } else {
        return (y >= T{})
            ? T(1) + ax / (ax + ay)
            : T(2) + ay / (ax + ay);
    }
}

template <Arithmetic T>
bool pseudoangle_less(const Vec<T, 2>& a, const Vec<T, 2>& b) {
    auto pa = pseudoangle(a);
    auto pb = pseudoangle(b);

    if (!pa.has_value()) return pb.has_value();
    if (!pb.has_value()) return false;

    return pa.value() < pb.value();
}

template <Arithmetic T>
std::expected<T, Error>
pseudoangleBetween(const Vec<T, 2>& a, const Vec<T, 2>& b) {
    auto pa = pseudoangle(a);
    auto pb = pseudoangle(b);

    if (!pa.has_value() || !pb.has_value())
        return std::unexpected(
            Error::make("One or both vectors are zero")
        );

    return pb.value() - pa.value();
}

}
