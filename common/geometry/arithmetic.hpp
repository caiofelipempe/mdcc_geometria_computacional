#pragma once
#pragma GCC target("avx")

#include <array>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <immintrin.h>

namespace geometry {

/* ================= CONCEITOS E ARMAZENAMENTO ================= */

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief ArithmeticVector: Define se o vetor terá tamanho fixo ou dinâmico.
 */
template <typename T, std::size_t N>
using ArithmeticVector = std::conditional_t<N == 0, std::vector<T>, std::array<T, N>>;

/* ================= UTILITÁRIOS DE APOIO ================= */

template <Arithmetic T, std::size_t N>
constexpr std::size_t getSize(const ArithmeticVector<T, N>& v) {
    if constexpr (N == 0) return v.size();
    else return N;
}

template <Arithmetic T, std::size_t N>
auto makeSimilar(const ArithmeticVector<T, N>& v) {
    if constexpr (N == 0) return std::vector<T>(v.size());
    else return std::array<T, N>{};
}

/* ================= SUPORTE SIMD ================= */

namespace simd {
    template <typename T>
    constexpr bool supported = std::is_same_v<T, float> || std::is_same_v<T, double>;

    struct Add {
        static __m128  v(__m128 a, __m128 b)   { return _mm_add_ps(a, b); }
        static __m256  v(__m256 a, __m256 b)   { return _mm256_add_ps(a, b); }
        static __m128d v(__m128d a, __m128d b) { return _mm_add_pd(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_add_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a + b; }
    };

    struct Sub {
        static __m128  v(__m128 a, __m128 b)   { return _mm_sub_ps(a, b); }
        static __m256  v(__m256 a, __m256 b)   { return _mm256_sub_ps(a, b); }
        static __m128d v(__m128d a, __m128d b) { return _mm_sub_pd(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_sub_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a - b; }
    };

    struct Mul {
        static __m128  v(__m128 a, __m128 b)   { return _mm_mul_ps(a, b); }
        static __m256  v(__m256 a, __m256 b)   { return _mm256_mul_ps(a, b); }
        static __m128d v(__m128d a, __m128d b) { return _mm_mul_pd(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_mul_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a * b; }
    };

    struct Div {
        static __m128  v(__m128 a, __m128 b)   { return _mm_div_ps(a, b); }
        static __m256  v(__m256 a, __m256 b)   { return _mm256_div_ps(a, b); }
        static __m128d v(__m128d a, __m128d b) { return _mm_div_pd(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_div_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a / b; }
    };

    template <typename Op, typename T>
    void apply_op(T* out, const T* a, const T* b, std::size_t n) {
        std::size_t i = 0;
        if constexpr (std::is_same_v<T, float>) {
#if defined(__AVX__)
            for (; i + 8 <= n; i += 8) 
                _mm256_storeu_ps(out + i, Op::v(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i)));
#endif
#if defined(__SSE__)
            for (; i + 4 <= n; i += 4) 
                _mm_storeu_ps(out + i, Op::v(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i)));
#endif
        } else if constexpr (std::is_same_v<T, double>) {
#if defined(__AVX__)
            for (; i + 4 <= n; i += 4) 
                _mm256_storeu_pd(out + i, Op::v(_mm256_loadu_pd(a + i), _mm256_loadu_pd(b + i)));
#endif
#if defined(__SSE__)
            for (; i + 2 <= n; i += 2) 
                _mm_storeu_pd(out + i, Op::v(_mm_loadu_pd(a + i), _mm_loadu_pd(b + i)));
#endif
        }
        for (; i < n; ++i) out[i] = Op::s(a[i], b[i]);
    }
}

/* ================= INTERFACE FUNCIONAL ================= */

template <Arithmetic T, std::size_t N>
auto add(const ArithmeticVector<T, N>& lhs, const ArithmeticVector<T, N>& rhs) {
    if constexpr (N == 0) if (lhs.size() != rhs.size()) throw std::runtime_error("Size mismatch");
    auto out = makeSimilar<T, N>(lhs);
    simd::apply_op<simd::Add>(out.data(), lhs.data(), rhs.data(), getSize<T, N>(lhs));
    return out;
}

template <Arithmetic T, std::size_t N>
auto sub(const ArithmeticVector<T, N>& lhs, const ArithmeticVector<T, N>& rhs) {
    if constexpr (N == 0) if (lhs.size() != rhs.size()) throw std::runtime_error("Size mismatch");
    auto out = makeSimilar<T, N>(lhs);
    simd::apply_op<simd::Sub>(out.data(), lhs.data(), rhs.data(), getSize<T, N>(lhs));
    return out;
}

template <Arithmetic T, std::size_t N>
T dot(const ArithmeticVector<T, N>& lhs, const ArithmeticVector<T, N>& rhs) {
    std::size_t n = getSize<T, N>(lhs);
    if constexpr (N == 0) if (n != rhs.size()) throw std::runtime_error("Size mismatch");
    
    T res{};
    std::size_t i = 0;
    const T* a = lhs.data();
    const T* b = rhs.data();

    if constexpr (std::is_same_v<T, float>) {
#if defined(__AVX__)
        __m256 acc = _mm256_setzero_ps();
        for (; i + 8 <= n; i += 8) acc = _mm256_add_ps(acc, _mm256_mul_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i)));
        alignas(32) float tmp[8]; _mm256_store_ps(tmp, acc);
        for (float v : tmp) res += v;
#elif defined(__SSE__)
        __m128 acc = _mm_setzero_ps();
        for (; i + 4 <= n; i += 4) acc = _mm_add_ps(acc, _mm_mul_ps(_mm_loadu_ps(a + i), _mm_loadu_ps(b + i)));
        alignas(16) float tmp[4]; _mm_store_ps(tmp, acc);
        for (float v : tmp) res += v;
#endif
    } else if constexpr (std::is_same_v<T, double>) {
#if defined(__AVX__)
        __m256d acc = _mm256_setzero_pd();
        for (; i + 4 <= n; i += 4) acc = _mm256_add_pd(acc, _mm256_mul_pd(_mm256_loadu_pd(a + i), _mm256_loadu_pd(b + i)));
        alignas(32) double tmp[4]; _mm256_store_pd(tmp, acc);
        for (double v : tmp) res += v;
#endif
    }
    for (; i < n; ++i) res += a[i] * b[i];
    return res;
}

/* ================= PRODUTO VETORIAL ================= */

template <Arithmetic T, std::size_t N> requires (N == 2)
T cross(const ArithmeticVector<T, 2>& a, const ArithmeticVector<T, 2>& b) {
    return a[0] * b[1] - a[1] * b[0];
}

template <Arithmetic T, std::size_t N> requires (N == 3)
auto cross(const ArithmeticVector<T, 3>& a, const ArithmeticVector<T, 3>& b) {
    return std::array<T, 3>{
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

template <Arithmetic T, std::size_t N> requires (N == 4)
auto cross(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, 4>& b) {
    return std::array<T, 4>{
        a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1],
        a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
        a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3],
        a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2]
    };
}

/* ====================== ROTATIONS ====================== */

template <Arithmetic T>
auto rotateWithQuaternion(const std::array<T, 3>& v, const std::array<T, 4>& q) {
    // q_vec = (x, y, z), q_w = w
    std::array<T, 3> q_vec = { q[0], q[1], q[2] };
    T q_w = q[3];

    std::array<T, 3> t = {
        static_cast<T>(2) * (q_vec[1] * v[2] - q_vec[2] * v[1]),
        static_cast<T>(2) * (q_vec[2] * v[0] - q_vec[0] * v[2]),
        static_cast<T>(2) * (q_vec[0] * v[1] - q_vec[1] * v[0])
    };

    return std::array<T, 3>{
        v[0] + q_w * t[0] + (q_vec[1] * t[2] - q_vec[2] * t[1]),
        v[1] + q_w * t[1] + (q_vec[2] * t[0] - q_vec[0] * t[2]),
        v[2] + q_w * t[2] + (q_vec[0] * t[1] - q_vec[1] * t[0])
    };
}

template <Arithmetic T>
auto rotateAxis(const std::array<T, 3>& vec, const std::array<T, 3>& axis, T angle_rad) {
    // 1. Construção do quatérnio a partir do eixo e ângulo
    T half_angle = angle_rad / static_cast<T>(2);
    T s = std::sin(half_angle);
    T c = std::cos(half_angle);

    std::array<T, 4> q = {
        axis[0] * s,
        axis[1] * s,
        axis[2] * s,
        c
    };

    return rotateWithQuaternion<T>(vec, q);
}

/* ================= OPERAÇÕES ESCALARES ================= */

template <Arithmetic T, std::size_t N>
auto div(const ArithmeticVector<T, N>& v, T scalar) {
    if (scalar == T{0}) throw std::runtime_error("Division by zero");
    auto out = makeSimilar<T, N>(v);
    std::size_t n = getSize<T, N>(v);
    std::size_t i = 0;
    if constexpr (simd::supported<T>) {
        if constexpr (std::is_same_v<T, float>) {
            __m256 s_vec = _mm256_set1_ps(scalar);
            for (; i + 8 <= n; i += 8) _mm256_storeu_ps(out.data() + i, _mm256_div_ps(_mm256_loadu_ps(v.data() + i), s_vec));
        } else if constexpr (std::is_same_v<T, double>) {
            __m256d s_vec = _mm256_set1_pd(scalar);
            for (; i + 4 <= n; i += 4) _mm256_storeu_pd(out.data() + i, _mm256_div_pd(_mm256_loadu_pd(v.data() + i), s_vec));
        }
    }
    for (; i < n; ++i) out[i] = v[i] / scalar;
    return out;
}

template <Arithmetic T, std::size_t N>
auto mul(const ArithmeticVector<T, N>& v, T scalar) {
    auto out = makeSimilar<T, N>(v);
    std::size_t n = getSize<T, N>(v);
    std::size_t i = 0;
    if constexpr (simd::supported<T>) {
        if constexpr (std::is_same_v<T, float>) {
            __m256 s_vec = _mm256_set1_ps(scalar);
            for (; i + 8 <= n; i += 8) _mm256_storeu_ps(out.data() + i, _mm256_mul_ps(_mm256_loadu_ps(v.data() + i), s_vec));
        } else if constexpr (std::is_same_v<T, double>) {
            __m256d s_vec = _mm256_set1_pd(scalar);
            for (; i + 4 <= n; i += 4) _mm256_storeu_pd(out.data() + i, _mm256_mul_pd(_mm256_loadu_pd(v.data() + i), s_vec));
        }
    }
    for (; i < n; ++i) out[i] = v[i] * scalar;
    return out;
}

/* ================= OPERADORES ================= */

template <Arithmetic T, std::size_t N>
auto operator+(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) { return add<T, N>(a, b); }

template <Arithmetic T, std::size_t N>
auto operator-(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) { return sub<T, N>(a, b); }

template <Arithmetic T, std::size_t N>
auto operator*(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) { 
    auto out = makeSimilar<T, N>(a);
    simd::apply_op<simd::Mul>(out.data(), a.data(), b.data(), getSize<T, N>(a));
    return out;
}

template <Arithmetic T, std::size_t N>
auto operator*(const ArithmeticVector<T, N>& v, T scalar) { return mul<T, N>(v, scalar); }

template <Arithmetic T, std::size_t N>
auto operator*(T scalar, const ArithmeticVector<T, N>& v) { return mul<T, N>(v, scalar); }

template <Arithmetic T, std::size_t N>
auto operator/(const ArithmeticVector<T, N>& v, T scalar) { return div<T, N>(v, scalar); }

template <Arithmetic T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const ArithmeticVector<T, N>& v) {
    os << "(";
    std::size_t n = getSize<T, N>(v);
    for (std::size_t i = 0; i < n; ++i) os << v[i] << (i + 1 < n ? ", " : "");
    return os << ")";
}

} // namespace geometry