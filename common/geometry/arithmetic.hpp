#pragma once
#pragma GCC target("avx,avx2,fma")

#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <immintrin.h>
#include <memory>

namespace geometry {

/* ================= CONCEITOS E ARMAZENAMENTO ================= */

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief Wrapper alinhado para garantir compatibilidade SIMD e silenciar warnings de atributos.
 */
template <typename T, std::size_t N>
struct alignas(32) AlignedArray {
    std::array<T, N> _data;
    
    // Interface de compatibilidade com std::array
    constexpr T* data() noexcept { return _data.data(); }
    constexpr const T* data() const noexcept { return _data.data(); }
    constexpr void fill(const T& value) { _data.fill(value); }
    constexpr std::size_t size() const noexcept { return N; }
    
    constexpr T& operator[](std::size_t i) { return _data[i]; }
    constexpr const T& operator[](std::size_t i) const { return _data[i]; }
    
    auto begin() { return _data.begin(); }
    auto end() { return _data.end(); }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }
};

// Alocador alinhado para std::vector (N=0)
template <typename T, std::size_t Align>
struct AlignedAllocator {
    using value_type = T;
    AlignedAllocator() = default;
    template <typename U> AlignedAllocator(const AlignedAllocator<U, Align>&) {}
    T* allocate(std::size_t n) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, Align, n * sizeof(T)) != 0) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t) { free(p); }
};

template <typename T, std::size_t N>
using ArithmeticVector = std::conditional_t<N == 0, 
    std::vector<T, AlignedAllocator<T, 32>>, 
    AlignedArray<T, N>>;

/* ================= UTILITÁRIOS DE APOIO ================= */

template <Arithmetic T, std::size_t N>
constexpr std::size_t getSize(const ArithmeticVector<T, N>& v) {
    if constexpr (N == 0) return v.size();
    else return N;
}

/**
 * @brief Cria uma estrutura similar para resultados intermediários.
 * Definida antes do uso para evitar erros de declaração.
 */
template <Arithmetic T, std::size_t N>
auto makeSimilar(const ArithmeticVector<T, N>& v) {
    if constexpr (N == 0) return std::vector<T, AlignedAllocator<T, 32>>(v.size());
    else return AlignedArray<T, N>{};
}

/* ================= SUPORTE SIMD AVANÇADO ================= */

namespace simd {
    inline float hsum(__m256 v) {
        __m128 lo = _mm256_castps256_ps128(v);
        __m128 hi = _mm256_extractf128_ps(v, 1);
        __m128 s = _mm_add_ps(lo, hi);
        s = _mm_add_ps(s, _mm_movehl_ps(s, s));
        s = _mm_add_ps(s, _mm_shuffle_ps(s, s, 0x55));
        return _mm_cvtss_f32(s);
    }

    inline double hsum(__m256d v) {
        __m128d lo = _mm256_castpd256_pd128(v);
        __m128d hi = _mm256_extractf128_pd(v, 1);
        __m128d s = _mm_add_pd(lo, hi);
        return _mm_cvtsd_f64(_mm_add_sd(s, _mm_unpackhi_pd(s, s)));
    }

    struct Add { 
        static __m256 v(__m256 a, __m256 b) { return _mm256_add_ps(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_add_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a + b; }
    };

    struct Sub { 
        static __m256 v(__m256 a, __m256 b) { return _mm256_sub_ps(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_sub_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a - b; }
    };

    struct Mul { 
        static __m256 v(__m256 a, __m256 b) { return _mm256_mul_ps(a, b); }
        static __m256d v(__m256d a, __m256d b) { return _mm256_mul_pd(a, b); }
        template<typename T> static T s(T a, T b) { return a * b; }
    };

    template <typename Op, typename T>
    void apply_op(T* out, const T* a, const T* b, std::size_t n) {
        std::size_t i = 0;
        if constexpr (std::is_same_v<T, float>) {
            for (; i + 8 <= n; i += 8)
                _mm256_store_ps(out + i, Op::v(_mm256_load_ps(a + i), _mm256_load_ps(b + i)));
        } else if constexpr (std::is_same_v<T, double>) {
            for (; i + 4 <= n; i += 4)
                _mm256_store_pd(out + i, Op::v(_mm256_load_pd(a + i), _mm256_load_pd(b + i)));
        }
        for (; i < n; ++i) out[i] = Op::s(a[i], b[i]);
    }
}

/* ================= INTERFACE FUNCIONAL ================= */

template <Arithmetic T, std::size_t N>
T dot(const ArithmeticVector<T, N>& lhs, const ArithmeticVector<T, N>& rhs) {
    std::size_t n = getSize<T, N>(lhs);
    const T* a = lhs.data();
    const T* b = rhs.data();
    std::size_t i = 0;
    T res = 0;

    if constexpr (std::is_same_v<T, float>) {
        __m256 acc = _mm256_setzero_ps();
        for (; i + 8 <= n; i += 8)
            acc = _mm256_fmadd_ps(_mm256_load_ps(a + i), _mm256_load_ps(b + i), acc);
        res = simd::hsum(acc);
    } else if constexpr (std::is_same_v<T, double>) {
        __m256d acc = _mm256_setzero_pd();
        for (; i + 4 <= n; i += 4)
            acc = _mm256_fmadd_pd(_mm256_load_pd(a + i), _mm256_load_pd(b + i), acc);
        res = simd::hsum(acc);
    }
    for (; i < n; ++i) res += a[i] * b[i];
    return res;
}

/* ================= PRODUTO VETORIAL OTIMIZADO ================= */

template <Arithmetic T, std::size_t N> requires (N == 3)
auto cross(const ArithmeticVector<T, 3>& a, const ArithmeticVector<T, 3>& b) {
    if constexpr (std::is_same_v<T, float>) {
        __m128 va = _mm_loadu_ps(a.data());
        __m128 vb = _mm_loadu_ps(b.data());
        __m128 res = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2))),
            _mm_mul_ps(_mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 0, 2, 1)))
        );
        AlignedArray<T, 3> out;
        _mm_storeu_ps(out.data(), res);
        return out;
    }
    AlignedArray<T, 3> out;
    out[0] = a[1]*b[2]-a[2]*b[1];
    out[1] = a[2]*b[0]-a[0]*b[2];
    out[2] = a[0]*b[1]-a[1]*b[0];
    return out;
}

/* ================= MÉTRICAS E NORMALIZAÇÃO ================= */

template <Arithmetic T, std::size_t N>
T sqrLength(const ArithmeticVector<T, N>& v) { return dot<T, N>(v, v); }

template <Arithmetic T, std::size_t N>
T length(const ArithmeticVector<T, N>& v) { return std::sqrt(sqrLength<T, N>(v)); }

template <Arithmetic T, std::size_t N>
auto mul(const ArithmeticVector<T, N>& v, T scalar);

template <Arithmetic T, std::size_t N>
auto normalize(const ArithmeticVector<T, N>& v) {
    T sql = sqrLength<T, N>(v);
    if (sql <= T{0}) throw std::runtime_error("Zero length");
    
    if constexpr (std::is_same_v<T, float>) {
        float r;
        __m128 s_sql = _mm_set_ss(sql);
        __m128 s_rsqrt = _mm_rsqrt_ss(s_sql);
        __m128 h = _mm_set_ss(0.5f), th = _mm_set_ss(1.5f);
        __m128 res = _mm_mul_ss(s_rsqrt, _mm_sub_ss(th, _mm_mul_ss(h, _mm_mul_ss(s_sql, _mm_mul_ss(s_rsqrt, s_rsqrt)))));
        _mm_store_ss(&r, res);
        return mul<T, N>(v, r);
    } else {
        return mul<T, N>(v, static_cast<T>(1) / std::sqrt(sql));
    }
}

/* ================= PROJEÇÃO E REFLEXÃO ================= */

template <Arithmetic T, std::size_t N>
auto project(const ArithmeticVector<T, N>& v, const ArithmeticVector<T, N>& nnit) {
    T d = dot<T, N>(v, nnit);
    return mul<T, N>(nnit, d);
}

template <Arithmetic T, std::size_t N>
auto reflect(const ArithmeticVector<T, N>& v, const ArithmeticVector<T, N>& nnit) {
    T d = dot<T, N>(v, nnit);
    return v - mul<T, N>(nnit, static_cast<T>(2) * d);
}

/* ================= ROTAÇÕES ================= */

template <Arithmetic T>
auto rotateWithQuaternion(const ArithmeticVector<T, 3>& v, const std::array<T, 4>& q) {
    if constexpr (std::is_same_v<T, float>) {
        __m128 v_simd = _mm_set_ps(0, v[2], v[1], v[0]);
        __m128 q_vec = _mm_loadu_ps(q.data()); 
        __m128 q_w = _mm_set1_ps(q[3]);

        auto cross_f = [](__m128 a, __m128 b) {
            return _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(a, a, 0xC9), _mm_shuffle_ps(b, b, 0xD2)),
                _mm_mul_ps(_mm_shuffle_ps(a, a, 0xD2), _mm_shuffle_ps(b, b, 0xC9))
            );
        };

        __m128 t = _mm_mul_ps(_mm_set1_ps(2.0f), cross_f(q_vec, v_simd));
        __m128 res = _mm_add_ps(v_simd, _mm_add_ps(_mm_mul_ps(q_w, t), cross_f(q_vec, t)));

        AlignedArray<T, 3> out;
        _mm_storeu_ps(out.data(), res);
        return out;
    }

    T tx = 2 * (q[1]*v[2] - q[2]*v[1]), ty = 2 * (q[2]*v[0] - q[0]*v[2]), tz = 2 * (q[0]*v[1] - q[1]*v[0]);
    AlignedArray<T, 3> out;
    out[0] = v[0] + q[3]*tx + (q[1]*tz - q[2]*ty);
    out[1] = v[1] + q[3]*ty + (q[2]*tx - q[0]*tz);
    out[2] = v[2] + q[3]*tz + (q[0]*ty - q[1]*tx);
    return out;
}

/* ================= OPERADORES ESCALARES ================= */

template <Arithmetic T, std::size_t N>
auto mul(const ArithmeticVector<T, N>& v, T scalar) {
    auto out = makeSimilar<T, N>(v);
    std::size_t n = getSize<T, N>(v), i = 0;
    if constexpr (std::is_same_v<T, float>) {
        __m256 s = _mm256_set1_ps(scalar);
        for (; i + 8 <= n; i += 8) _mm256_store_ps(out.data() + i, _mm256_mul_ps(_mm256_load_ps(v.data() + i), s));
    } else if constexpr (std::is_same_v<T, double>) {
        __m256d s = _mm256_set1_pd(scalar);
        for (; i + 4 <= n; i += 4) _mm256_store_pd(out.data() + i, _mm256_mul_pd(_mm256_load_pd(v.data() + i), s));
    }
    for (; i < n; ++i) out[i] = v[i] * scalar;
    return out;
}

/* ================= SOBRECARGAS IN-PLACE ================= */

template <Arithmetic T, std::size_t N>
void operator+=(ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) {
    simd::apply_op<simd::Add>(a.data(), a.data(), b.data(), getSize<T, N>(a));
}

template <Arithmetic T, std::size_t N>
void operator-=(ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) {
    simd::apply_op<simd::Sub>(a.data(), a.data(), b.data(), getSize<T, N>(a));
}

/* ================= SOBRECARGAS PADRÃO ================= */

template <Arithmetic T, std::size_t N>
auto operator+(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) {
    auto out = makeSimilar<T, N>(a);
    simd::apply_op<simd::Add>(out.data(), a.data(), b.data(), getSize<T, N>(a));
    return out;
}

template <Arithmetic T, std::size_t N>
auto operator-(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) {
    auto out = makeSimilar<T, N>(a);
    simd::apply_op<simd::Sub>(out.data(), a.data(), b.data(), getSize<T, N>(a));
    return out;
}

template <Arithmetic T, std::size_t N>
auto operator*(const ArithmeticVector<T, N>& a, const ArithmeticVector<T, N>& b) {
    auto out = makeSimilar<T, N>(a);
    simd::apply_op<simd::Mul>(out.data(), a.data(), b.data(), getSize<T, N>(a));
    return out;
}

template <Arithmetic T, std::size_t N>
auto operator*(const ArithmeticVector<T, N>& v, T s) { return mul<T, N>(v, s); }

template <Arithmetic T, std::size_t N>
auto operator*(T s, const ArithmeticVector<T, N>& v) { return mul<T, N>(v, s); }

} // namespace geometry