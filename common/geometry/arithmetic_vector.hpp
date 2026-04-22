#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "error.hpp"
#include "result.hpp"
#include "vec_error.hpp"

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T, std::size_t N>
using VectorOrArray =
    std::conditional_t<N == 0, std::vector<T>, std::array<T, N>>;

template <Arithmetic T, std::size_t N>
class ArithmeticVector {
public:
    using ErrorType = Error<VecError>;
    using Vec      = ArithmeticVector<T, N>;

    VectorOrArray<T, N> data;

    /* ================= CONSTRUTORES ================= */

    ArithmeticVector() = default;

    ArithmeticVector(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N)
                throw std::invalid_argument("Dimensão incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    explicit ArithmeticVector(std::size_t size) {
        static_assert(N == 0);
        data.resize(size);
    }

    /* ================= ACESSO ================= */

    T& operator[](std::size_t i) { return data[i]; }
    const T& operator[](std::size_t i) const { return data[i]; }

    std::size_t size() const {
        if constexpr (N == 0) return data.size();
        else return N;
    }

private:
    /* ===== Runtime check SOMENTE para N == 0 ===== */

    static void check_runtime_size(const Vec& a, const Vec& b) {
        if constexpr (N == 0) {
            if (a.size() != b.size())
                throw std::runtime_error("Dimensões incompatíveis");
        }
    }

    static Vec make_result(const Vec& ref) {
        if constexpr (N == 0) return Vec(ref.size());
        else return Vec{};
    }

    static constexpr T default_epsilon() noexcept {
        if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(1e-8);
        else
            return T{};
    }

public:
    /* ================= VETOR × VETOR ================= */

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    add(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] + rhs[i];

        return out;
    }

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    sub(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] - rhs[i];

        return out;
    }

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    mul(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] * rhs[i];

        return out;
    }

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    div(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        for (std::size_t i = 0; i < size(); ++i) {
            if (rhs[i] == T{}) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::DivisionByZero)
                    );
                else
                    throw std::runtime_error("Division by zero");
            }
        }

        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] / rhs[i];

        return out;
    }

    /* ================= ESCALAR ================= */

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    mul(T scalar) const {
        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] * scalar;
        return out;
    }

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    div(T scalar) const {
        if (scalar == T{}) {
            if constexpr (Safe)
                return std::unexpected(
                    ErrorType::make(VecError::DivisionByZero)
                );
            else
                throw std::runtime_error("Division by zero");
        }

        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] / scalar;

        return out;
    }

    /* ================= MATEMÁTICA ================= */

    template <bool Safe = true>
    Result<T, ErrorType, Safe>
    dot(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        ErrorType::make(VecError::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        T sum{};
        for (std::size_t i = 0; i < size(); ++i)
            sum += data[i] * rhs[i];

        return sum;
    }

    T sqr_norm() const {
        T s{};
        for (std::size_t i = 0; i < size(); ++i)
            s += data[i] * data[i];
        return s;
    }

    template <bool Safe = true>
    Result<T, ErrorType, Safe>
    norm() const {
        T sn = sqr_norm();
        if (sn == T{}) {
            if constexpr (Safe)
                return std::unexpected(
                    ErrorType::make(VecError::ZeroNorm)
                );
            else
                throw std::runtime_error("Zero norm");
        }
        return std::sqrt(sn);
    }

    template <bool Safe = true>
    Result<Vec, ErrorType, Safe>
    normalized(T eps = default_epsilon()) const {
        T sn = sqr_norm();
        if (sn < eps * eps) {
            if constexpr (Safe)
                return std::unexpected(
                    ErrorType::make(VecError::ZeroNorm)
                );
            else
                throw std::runtime_error("Zero norm");
        }
        return *this / std::sqrt(sn);
    }

    /* ================= OPERADORES (UNSAFE) ================= */

    Vec operator+(const Vec& rhs) const { return add<false>(rhs); }
    Vec operator-(const Vec& rhs) const { return sub<false>(rhs); }
    Vec operator*(const Vec& rhs) const { return mul<false>(rhs); }
    Vec operator*(T s) const           { return mul<false>(s);   }
    Vec operator/(T s) const           { return div<false>(s);   }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Vec& v) {
        os << "(";
        for (std::size_t i = 0; i < v.size(); ++i) {
            os << v[i];
            if (i + 1 < v.size()) os << ", ";
        }
        os << ")";
        return os;
    }
};