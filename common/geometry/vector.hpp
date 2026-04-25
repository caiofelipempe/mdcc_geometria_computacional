#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <immintrin.h>
#include <span>

#include "result.hpp"

namespace geometry {

/* ================= CONCEPT ================= */

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/* ================= STORAGE ================= */

template <typename T, std::size_t N>
using VectorOrArray =
    std::conditional_t<N == 0, std::vector<T>, std::array<T, N>>;

/* ================= SIMD SUPPORT ================= */

namespace simd {

template <typename T>
constexpr bool supported =
    std::is_same_v<T, float> ||
    std::is_same_v<T, double>;

}

/* ================= VECTOR ================= */

template <Arithmetic T, std::size_t N>
class Vector {
public:
    enum class Error {
        SizeMismatch,
        DivisionByZero,
        ZeroNorm
    };

    using Vec = Vector<T, N>;

    VectorOrArray<T, N> data;

    /* ================= CONSTRUCTORS ================= */

    Vector() = default;

    Vector(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N)
                throw std::invalid_argument("Dimensão incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    explicit Vector(std::size_t size) {
        static_assert(N == 0);
        data.resize(size);
    }

    /* ================= SIZE ================= */
    
    std::size_t size() const noexcept {
        if constexpr (N == 0)
            return data.size();
        else
            return N;
    }

    /* ================= ACCESS ================= */

    std::span<T> span() noexcept {
        return { data_ptr(), size() };
    }

    std::span<const T> span() const noexcept {
        return { data_ptr(), size() };
    }

    /* ================= ACCESS ================= */

    T& operator[](std::size_t i) {
        if constexpr (N == 0) {
            if (i >= data.size())
                throw std::out_of_range("Vector index out of range");
        }
        return data[i];
    }

    const T& operator[](std::size_t i) const {
        if constexpr (N == 0) {
            if (i >= data.size())
                throw std::out_of_range("Vector index out of range");
        }
        return data[i];
    }

    /* ================= SIMD ACCESS ================= */

    T* data_ptr() noexcept {
        return data.data();
    }

    const T* data_ptr() const noexcept {
        return data.data();
    }

private:
    static Vec make_result(const Vec& ref) {
        if constexpr (N == 0) return Vec(ref.size());
        else return Vec{};
    }

    /* ================= SIMD OPS ================= */

    struct AddOp {
        static T scalar(T a, T b) { return a + b; }
        static __m128  apply(__m128 a, __m128 b)   { return _mm_add_ps(a,b); }
        static __m256  apply(__m256 a, __m256 b)   { return _mm256_add_ps(a,b); }
        static __m128d apply(__m128d a, __m128d b) { return _mm_add_pd(a,b); }
        static __m256d apply(__m256d a, __m256d b) { return _mm256_add_pd(a,b); }
    };

    struct SubOp {
        static T scalar(T a, T b) { return a - b; }
        static __m128  apply(__m128 a, __m128 b)   { return _mm_sub_ps(a,b); }
        static __m256  apply(__m256 a, __m256 b)   { return _mm256_sub_ps(a,b); }
        static __m128d apply(__m128d a, __m128d b) { return _mm_sub_pd(a,b); }
        static __m256d apply(__m256d a, __m256d b) { return _mm256_sub_pd(a,b); }
    };

    struct MulOp {
        static T scalar(T a, T b) { return a * b; }
        static __m128  apply(__m128 a, __m128 b)   { return _mm_mul_ps(a,b); }
        static __m256  apply(__m256 a, __m256 b)   { return _mm256_mul_ps(a,b); }
        static __m128d apply(__m128d a, __m128d b) { return _mm_mul_pd(a,b); }
        static __m256d apply(__m256d a, __m256d b) { return _mm256_mul_pd(a,b); }
    };

    /* SIMD APPLY CORE */
    template <typename Op>
    static void simd_apply(
        T* out,
        const T* a,
        const T* b,
        std::size_t n
    ) {
        std::size_t i = 0;

        if constexpr (std::is_same_v<T, float>) {
#if defined(__AVX__)
            for (; i + 8 <= n; i += 8) {
                __m256 va = _mm256_loadu_ps(a + i);
                __m256 vb = _mm256_loadu_ps(b + i);
                _mm256_storeu_ps(out + i, Op::apply(va, vb));
            }
#endif
#if defined(__SSE__)
            for (; i + 4 <= n; i += 4) {
                __m128 va = _mm_loadu_ps(a + i);
                __m128 vb = _mm_loadu_ps(b + i);
                _mm_storeu_ps(out + i, Op::apply(va, vb));
            }
#endif
        }

        if constexpr (std::is_same_v<T, double>) {
#if defined(__AVX__)
            for (; i + 4 <= n; i += 4) {
                __m256d va = _mm256_loadu_pd(a + i);
                __m256d vb = _mm256_loadu_pd(b + i);
                _mm256_storeu_pd(out + i, Op::apply(va, vb));
            }
#endif
#if defined(__SSE__)
            for (; i + 2 <= n; i += 2) {
                __m128d va = _mm_loadu_pd(a + i);
                __m128d vb = _mm_loadu_pd(b + i);
                _mm_storeu_pd(out + i, Op::apply(va, vb));
            }
#endif
        }

        for (; i < n; ++i)
            out[i] = Op::scalar(a[i], b[i]);
    }

public:
    /* ================= OPERATIONS ================= */

    Vec add(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size())
                throw std::runtime_error("Size mismatch");
        }

        Vec out = make_result(*this);

        if constexpr (simd::supported<T>) {
            simd_apply<AddOp>(
                out.data_ptr(),
                data_ptr(),
                rhs.data_ptr(),
                size()
            );
        } else {
            for (std::size_t i = 0; i < size(); ++i)
                out[i] = data[i] + rhs[i];
        }

        return out;
    }

    Vec sub(const Vec& rhs) const {
        Vec out = make_result(*this);
        simd_apply<SubOp>(out.data_ptr(), data_ptr(), rhs.data_ptr(), size());
        return out;
    }

    Vec mul(const Vec& rhs) const {
        Vec out = make_result(*this);
        simd_apply<MulOp>(out.data_ptr(), data_ptr(), rhs.data_ptr(), size());
        return out;
    }

    /* ================= OPERATORS ================= */

    Vec operator+(const Vec& rhs) const { return add(rhs); }
    Vec operator-(const Vec& rhs) const { return sub(rhs); }
    Vec operator*(const Vec& rhs) const { return mul(rhs); }

    friend std::ostream& operator<<(std::ostream& os, const Vec& v) {
        os << "(";
        for (std::size_t i = 0; i < v.size(); ++i) {
            os << v[i];
            if (i + 1 < v.size()) os << ", ";
        }
        os << ")";
        return os;
    }

    /* ================= ESCALAR ================= */

    Vec operator*(T scalar) const {
        Vec out = make_result(*this);

        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] * scalar;

        return out;
    }

    Vec operator/(T scalar) const {
        if (scalar == T{})
            throw std::runtime_error("Division by zero");

        Vec out = make_result(*this);

        for (std::size_t i = 0; i < size(); ++i)
            out[i] = data[i] / scalar;

        return out;
    }

    /* ================= LINEAR OPERATIONS ================= */

    T dot(const Vec& rhs) const {
        if constexpr (N == 0) {
            if (size() != rhs.size())
                throw std::runtime_error("Size mismatch");
        }

        T result{};

        if constexpr (geometry::simd::supported<T>) {

            std::size_t i = 0;

            if constexpr (std::is_same_v<T, float>) {

#if defined(__AVX__)
                __m256 acc = _mm256_setzero_ps();
                for (; i + 8 <= size(); i += 8) {
                    __m256 a = _mm256_loadu_ps(data_ptr() + i);
                    __m256 b = _mm256_loadu_ps(rhs.data_ptr() + i);
                    acc = _mm256_add_ps(acc, _mm256_mul_ps(a, b));
                }

                alignas(32) float tmp[8];
                _mm256_store_ps(tmp, acc);
                for (float v : tmp) result += v;
#endif

#if defined(__SSE__)
                __m128 acc = _mm_setzero_ps();
                for (; i + 4 <= size(); i += 4) {
                    __m128 a = _mm_loadu_ps(data_ptr() + i);
                    __m128 b = _mm_loadu_ps(rhs.data_ptr() + i);
                    acc = _mm_add_ps(acc, _mm_mul_ps(a, b));
                }

                alignas(16) float tmp[4];
                _mm_store_ps(tmp, acc);
                for (float v : tmp) result += v;
#endif
            }

            if constexpr (std::is_same_v<T, double>) {

#if defined(__AVX__)
                __m256d acc = _mm256_setzero_pd();
                for (; i + 4 <= size(); i += 4) {
                    __m256d a = _mm256_loadu_pd(data_ptr() + i);
                    __m256d b = _mm256_loadu_pd(rhs.data_ptr() + i);
                    acc = _mm256_add_pd(acc, _mm256_mul_pd(a, b));
                }

                alignas(32) double tmp[4];
                _mm256_store_pd(tmp, acc);
                for (double v : tmp) result += v;
#endif

#if defined(__SSE__)
                __m128d acc = _mm_setzero_pd();
                for (; i + 2 <= size(); i += 2) {
                    __m128d a = _mm_loadu_pd(data_ptr() + i);
                    __m128d b = _mm_loadu_pd(rhs.data_ptr() + i);
                    acc = _mm_add_pd(acc, _mm_mul_pd(a, b));
                }

                alignas(16) double tmp[2];
                _mm_store_pd(tmp, acc);
                for (double v : tmp) result += v;
#endif
            }

            // resto escalar
            for (; i < size(); ++i)
                result += data[i] * rhs[i];

        } else {
            // fallback escalar
            for (std::size_t i = 0; i < size(); ++i)
                result += data[i] * rhs[i];
        }

        return result;
    }

    template <std::size_t M = N>
    requires (M == 2)
    T cross(const Vec& rhs) const {
        return data[0] * rhs[1] - data[1] * rhs[0];
    }

    template <std::size_t M = N>
    requires (M == 3)
    Vec cross(const Vec& rhs) const {
        return Vec{
            data[1] * rhs[2] - data[2] * rhs[1],
            data[2] * rhs[0] - data[0] * rhs[2],
            data[0] * rhs[1] - data[1] * rhs[0]
        };
    }

    template <std::size_t M = N>
    requires (M == 4)
    Vec cross(const Vec& b, const Vec& c) const {
        const Vec& a = *this;
        Vec out{};

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

        out[3] =
            -(a[0] * (b[1] * c[2] - b[2] * c[1])
            - a[1] * (b[0] * c[2] - b[2] * c[0])
            + a[2] * (b[0] * c[1] - b[1] * c[0]));

        return out;
    }

    Vec normalized(T eps = static_cast<T>(1e-8)) const {
        // ||v||^2
        T sqr = dot(*this);

        if (sqr <= eps * eps)
            throw std::runtime_error("Zero norm");

        T inv_len = static_cast<T>(1) / std::sqrt(sqr);

        Vec out = make_result(*this);

        if constexpr (geometry::simd::supported<T>) {

            std::size_t i = 0;

            if constexpr (std::is_same_v<T, float>) {

#if defined(__AVX__)
                __m256 s = _mm256_set1_ps(inv_len);
                for (; i + 8 <= size(); i += 8) {
                    __m256 v = _mm256_loadu_ps(data_ptr() + i);
                    _mm256_storeu_ps(out.data_ptr() + i,
                                    _mm256_mul_ps(v, s));
                }
#endif
#if defined(__SSE__)
                __m128 s = _mm_set1_ps(inv_len);
                for (; i + 4 <= size(); i += 4) {
                    __m128 v = _mm_loadu_ps(data_ptr() + i);
                    _mm_storeu_ps(out.data_ptr() + i,
                                _mm_mul_ps(v, s));
                }
#endif
            }

            if constexpr (std::is_same_v<T, double>) {

#if defined(__AVX__)
                __m256d s = _mm256_set1_pd(inv_len);
                for (; i + 4 <= size(); i += 4) {
                    __m256d v = _mm256_loadu_pd(data_ptr() + i);
                    _mm256_storeu_pd(out.data_ptr() + i,
                                    _mm256_mul_pd(v, s));
                }
#endif
#if defined(__SSE__)
                __m128d s = _mm_set1_pd(inv_len);
                for (; i + 2 <= size(); i += 2) {
                    __m128d v = _mm_loadu_pd(data_ptr() + i);
                    _mm_storeu_pd(out.data_ptr() + i,
                                _mm_mul_pd(v, s));
                }
#endif
            }

            // resto escalar
            for (; i < size(); ++i)
                out[i] = data[i] * inv_len;

        } else {
            // fallback escalar
            for (std::size_t i = 0; i < size(); ++i)
                out[i] = data[i] * inv_len;
        }

        return out;
    }

};

} // namespace geometry