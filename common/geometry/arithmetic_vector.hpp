#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <random>

#include "error.hpp"
#include "result.hpp"
#include "error.hpp"

namespace geometry {

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T, std::size_t N>
using VectorOrArray =
    std::conditional_t<N == 0, std::vector<T>, std::array<T, N>>;

template <Arithmetic T, std::size_t N>
class ArithmeticVector {
public:

    enum class ErrCode {
        SizeMismatch,
        DivisionByZero,
        ZeroNorm
    };

    using Err = Error<ErrCode>;
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

    /* ================= EPISILON ================= */


    static constexpr T defaultEpsilon() noexcept {
        if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(1e-8);
        else
            return T{};
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
    Result<Vec, Err, Safe>
    add(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(Err::make(ErrCode::SizeMismatch));
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
    Result<Vec, Err, Safe>
    sub(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        Err::make(ErrCode::SizeMismatch)
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
    Result<Vec, Err, Safe>
    mul(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        Err::make(ErrCode::SizeMismatch)
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
    Result<Vec, Err, Safe>
    div(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size()) {
                if constexpr (Safe)
                    return std::unexpected(
                        Err::make(ErrCode::SizeMismatch)
                    );
                else
                    throw std::runtime_error("Size mismatch");
            }
        }

        for (std::size_t i = 0; i < size(); ++i) {
            if (rhs[i] == T{}) {
                if constexpr (Safe)
                    return std::unexpected(
                        Err::make(ErrCode::DivisionByZero)
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
    Result<Vec, Err, Safe>
    mul(T scalar) const {
        Vec out = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] * scalar;
        return out;
    }

    template <bool Safe = true>
    Result<Vec, Err, Safe>
    div(T scalar) const {
        if (scalar == T{}) {
            if constexpr (Safe)
                return std::unexpected(
                    Err::make(ErrCode::DivisionByZero)
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

    T dot(const Vec& rhs) const {
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
    Result<T, Err, Safe>
    norm() const {
        T sn = sqr_norm();
        if (sn == T{}) {
            if constexpr (Safe)
                return std::unexpected(
                    Err::make(ErrCode::ZeroNorm)
                );
            else
                throw std::runtime_error("Zero norm");
        }
        return std::sqrt(sn);
    }

    template <bool Safe = true>
    Result<Vec, Err, Safe>
    normalized(T eps = default_epsilon()) const {
        T sn = sqr_norm();
        if (sn < eps * eps) {
            if constexpr (Safe)
                return std::unexpected(
                    Err::make(ErrCode::ZeroNorm)
                );
            else
                throw std::runtime_error("Zero norm");
        }
        return *this / std::sqrt(sn);
    }


    template <bool Safe = true>
    requires (N == 2)
    T cross(const Vec& rhs) const {
        return data[0] * rhs[1] - data[1] * rhs[0];
    }

    /* ---------- 3D: cross binário clássico ---------- */
    template <bool Safe = true>
    requires (N == 3)
    Vec cross(const Vec& rhs) const {
        Vec out{};
        out[0] = data[1] * rhs[2] - data[2] * rhs[1];
        out[1] = data[2] * rhs[0] - data[0] * rhs[2];
        out[2] = data[0] * rhs[1] - data[1] * rhs[0];
        return out;
    }

    template <bool Safe = true>
    requires (N == 4)
    Vec cross(const Vec& b, const Vec& c) const {
        const Vec& a = *this;

        Vec out{};

        // Parte vetorial (cross 3D clássico usando xyz)
        out[0] =
            +(a[1] * (b[2] * c[3] - b[3] * c[2])
            - a[2] * (b[1] * c[3] - b[3] * c[1])
            + a[3] * (b[1] * c[2] - b[2] * c[1]));

        out[1] =
            -(a[0] * (b[2] * c[3] - b[3] * c[2])
            - a[2] * (b[0] * c[3] - b[3] * c[0])
            + a[3] * (b[0] * c[2] - b[2] * c[0]));

        out[2] =
            +(a[0] * (b[1] * c[3] - b[3] * c[1])
            - a[1] * (b[0] * c[3] - b[3] * c[0])
            + a[3] * (b[0] * c[1] - b[1] * c[0]));

        // Parte real (determinante do volume tridimensional)
        out[3] =
            -(a[0] * (b[1] * c[2] - b[2] * c[1])
            - a[1] * (b[0] * c[2] - b[2] * c[0])
            + a[2] * (b[0] * c[1] - b[1] * c[0]));

        return out;
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

}

template <geometry::Arithmetic T, std::size_t N>
geometry::ArithmeticVector<T, N>
operator*(T s, const geometry::ArithmeticVector<T, N>& v) {
    return v * s;
}
