#pragma once
#pragma GCC target("avx,avx2,fma")

#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <immintrin.h>
#include <memory>

namespace geometry {

/* ================= CONCEITOS ================= */

template<typename T>
concept Addable =
requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Subtractable =
requires(T a, T b) {
    { a - b } -> std::convertible_to<T>;
};

template<typename T>
concept Multipliable =
requires(T a, T b) {
    { a * b } -> std::convertible_to<T>;
};

template<typename T>
concept Divisible =
requires(T a, T b) {
    { a / b } -> std::convertible_to<T>;
};

template<typename T>
concept Negatable =
requires(T a) {
    { -a } -> std::convertible_to<T>;
};

template<typename T>
concept Comparable =
requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
    { a <  b } -> std::convertible_to<bool>;
    { a <= b } -> std::convertible_to<bool>;
    { a >  b } -> std::convertible_to<bool>;
    { a >= b } -> std::convertible_to<bool>;
};

template<typename T>
concept DefaultConstructible =
requires {
    T{};
};

template<typename T>
concept Sqrtable =
requires(T a) {
    { std::sqrt(a) } -> std::convertible_to<T>;
};

template<typename T>
concept Scalar =
    Addable<T> &&
    Subtractable<T> &&
    Multipliable<T> &&
    Divisible<T> &&
    Negatable<T> &&
    Comparable<T> &&
    DefaultConstructible<T>;

template<typename T>
concept NormalizableScalar =
    Scalar<T> &&
    Sqrtable<T>;

/* ================= ARMAZENAMENTO ================= */

template <typename T, std::size_t N>
struct alignas(32) AlignedArray {
    std::array<T, N> _data;

    constexpr T* data() noexcept { return _data.data(); }
    constexpr const T* data() const noexcept { return _data.data(); }

    constexpr void fill(const T& value) {
        _data.fill(value);
    }

    constexpr std::size_t size() const noexcept {
        return N;
    }

    constexpr T& operator[](std::size_t i) {
        return _data[i];
    }

    constexpr const T& operator[](std::size_t i) const {
        return _data[i];
    }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end(); }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }
};

/* ================= ALOCADOR ALINHADO ================= */

template <typename T, std::size_t Align>
struct AlignedAllocator {
    using value_type = T;

    AlignedAllocator() = default;

    template<typename U>
    constexpr AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept {}

    [[nodiscard]]
    T* allocate(std::size_t n) {
        void* ptr = nullptr;

        if (posix_memalign(&ptr, Align, n * sizeof(T)) != 0)
            throw std::bad_alloc();

        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
        free(p);
    }
};

template<typename T1, typename T2, std::size_t A>
constexpr bool operator==(const AlignedAllocator<T1, A>&,
                          const AlignedAllocator<T2, A>&) noexcept {
    return true;
}

/* ================= VECTOR STORAGE ================= */

template <typename T, std::size_t N>
using LinearStorage =
    std::conditional_t<
        N == 0,
        std::vector<T, AlignedAllocator<T, 32>>,
        AlignedArray<T, N>
    >;

/* ================= UTILITÁRIOS ================= */

template <Scalar T, std::size_t N>
constexpr std::size_t getSize(const LinearStorage<T, N>& v) {
    if constexpr (N == 0)
        return v.size();
    else
        return N;
}

template <Scalar T, std::size_t N>
auto makeSimilar(const LinearStorage<T, N>& v) {
    if constexpr (N == 0)
        return std::vector<T, AlignedAllocator<T, 32>>(v.size());
    else
        return AlignedArray<T, N>{};
}

/* ================= SIMD ================= */

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

    return _mm_cvtsd_f64(
        _mm_add_sd(s, _mm_unpackhi_pd(s, s))
    );
}

struct Add {
    static __m256 v(__m256 a, __m256 b) {
        return _mm256_add_ps(a, b);
    }

    static __m256d v(__m256d a, __m256d b) {
        return _mm256_add_pd(a, b);
    }

    template<typename T>
    static T s(const T& a, const T& b) {
        return a + b;
    }
};

struct Sub {
    static __m256 v(__m256 a, __m256 b) {
        return _mm256_sub_ps(a, b);
    }

    static __m256d v(__m256d a, __m256d b) {
        return _mm256_sub_pd(a, b);
    }

    template<typename T>
    static T s(const T& a, const T& b) {
        return a - b;
    }
};

struct Mul {
    static __m256 v(__m256 a, __m256 b) {
        return _mm256_mul_ps(a, b);
    }

    static __m256d v(__m256d a, __m256d b) {
        return _mm256_mul_pd(a, b);
    }

    template<typename T>
    static T s(const T& a, const T& b) {
        return a * b;
    }
};

template <typename Op, typename T>
void apply_op(T* out,
              const T* a,
              const T* b,
              std::size_t n) {

    std::size_t i = 0;

    if constexpr (std::is_same_v<T, float>) {

        for (; i + 8 <= n; i += 8) {

            _mm256_store_ps(
                out + i,
                Op::v(
                    _mm256_load_ps(a + i),
                    _mm256_load_ps(b + i)
                )
            );
        }

    } else if constexpr (std::is_same_v<T, double>) {

        for (; i + 4 <= n; i += 4) {

            _mm256_store_pd(
                out + i,
                Op::v(
                    _mm256_load_pd(a + i),
                    _mm256_load_pd(b + i)
                )
            );
        }
    }

    for (; i < n; ++i)
        out[i] = Op::s(a[i], b[i]);
}

}

/* ================= DOT PRODUCT ================= */

template <Scalar T, std::size_t N>
T dot(const LinearStorage<T, N>& lhs,
      const LinearStorage<T, N>& rhs) {

    std::size_t n = getSize<T, N>(lhs);

    const T* a = lhs.data();
    const T* b = rhs.data();

    std::size_t i = 0;

    T res{};

    if constexpr (std::is_same_v<T, float>) {

        __m256 acc = _mm256_setzero_ps();

        for (; i + 8 <= n; i += 8) {

            acc = _mm256_fmadd_ps(
                _mm256_load_ps(a + i),
                _mm256_load_ps(b + i),
                acc
            );
        }

        res = simd::hsum(acc);

    } else if constexpr (std::is_same_v<T, double>) {

        __m256d acc = _mm256_setzero_pd();

        for (; i + 4 <= n; i += 4) {

            acc = _mm256_fmadd_pd(
                _mm256_load_pd(a + i),
                _mm256_load_pd(b + i),
                acc
            );
        }

        res = simd::hsum(acc);
    }

    for (; i < n; ++i)
        res = res + (a[i] * b[i]);

    return res;
}

/* ================= CROSS PRODUCT ================= */

template <Scalar T, std::size_t N>
requires (N == 3)
auto cross(const LinearStorage<T, 3>& a,
           const LinearStorage<T, 3>& b) {

    if constexpr (std::is_same_v<T, float>) {

        __m128 va = _mm_loadu_ps(a.data());
        __m128 vb = _mm_loadu_ps(b.data());

        __m128 res = _mm_sub_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(va, va, _MM_SHUFFLE(3,0,2,1)),
                _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3,1,0,2))
            ),
            _mm_mul_ps(
                _mm_shuffle_ps(va, va, _MM_SHUFFLE(3,1,0,2)),
                _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3,0,2,1))
            )
        );

        AlignedArray<T, 3> out;

        _mm_storeu_ps(out.data(), res);

        return out;
    }

    AlignedArray<T, 3> out;

    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];

    return out;
}

/* ================= MÉTRICAS ================= */

template <Scalar T, std::size_t N>
T sqrLength(const LinearStorage<T, N>& v) {
    return dot<T, N>(v, v);
}

template <NormalizableScalar T, std::size_t N>
T length(const LinearStorage<T, N>& v) {
    return std::sqrt(sqrLength<T, N>(v));
}

/* ================= ESCALAR ================= */

template <Scalar T, std::size_t N>
auto mul(const LinearStorage<T, N>& v,
         T scalar) {

    auto out = makeSimilar<T, N>(v);

    std::size_t n = getSize<T, N>(v);

    std::size_t i = 0;

    if constexpr (std::is_same_v<T, float>) {

        __m256 s = _mm256_set1_ps(scalar);

        for (; i + 8 <= n; i += 8) {

            _mm256_store_ps(
                out.data() + i,
                _mm256_mul_ps(
                    _mm256_load_ps(v.data() + i),
                    s
                )
            );
        }

    } else if constexpr (std::is_same_v<T, double>) {

        __m256d s = _mm256_set1_pd(scalar);

        for (; i + 4 <= n; i += 4) {

            _mm256_store_pd(
                out.data() + i,
                _mm256_mul_pd(
                    _mm256_load_pd(v.data() + i),
                    s
                )
            );
        }
    }

    for (; i < n; ++i)
        out[i] = v[i] * scalar;

    return out;
}

/* ================= NORMALIZAÇÃO ================= */

template <NormalizableScalar T, std::size_t N>
auto normalize(const LinearStorage<T, N>& v) {

    T sql = sqrLength<T, N>(v);

    if (sql <= T{})
        throw std::runtime_error("Zero length");

    if constexpr (std::is_same_v<T, float>) {

        float r;

        __m128 s_sql = _mm_set_ss(sql);

        __m128 s_rsqrt = _mm_rsqrt_ss(s_sql);

        __m128 h = _mm_set_ss(0.5f);
        __m128 th = _mm_set_ss(1.5f);

        __m128 res = _mm_mul_ss(
            s_rsqrt,
            _mm_sub_ss(
                th,
                _mm_mul_ss(
                    h,
                    _mm_mul_ss(
                        s_sql,
                        _mm_mul_ss(s_rsqrt, s_rsqrt)
                    )
                )
            )
        );

        _mm_store_ss(&r, res);

        return mul<T, N>(v, r);

    } else {

        return mul<T, N>(
            v,
            T{1} / std::sqrt(sql)
        );
    }
}

/* ================= PROJEÇÃO ================= */

template <Scalar T, std::size_t N>
auto project(const LinearStorage<T, N>& v,
             const LinearStorage<T, N>& nnit) {

    T d = dot<T, N>(v, nnit);

    return mul<T, N>(nnit, d);
}

/* ================= REFLEXÃO ================= */

template <Scalar T, std::size_t N>
auto reflect(const LinearStorage<T, N>& v,
             const LinearStorage<T, N>& nnit) {

    T d = dot<T, N>(v, nnit);

    return v - mul<T, N>(nnit, T{2} * d);
}

/* ================= QUATERNION ================= */

template <Scalar T>
auto rotateWithQuaternion(const LinearStorage<T, 3>& v,
                          const std::array<T, 4>& q) {

    if constexpr (std::is_same_v<T, float>) {

        __m128 v_simd =
            _mm_set_ps(0, v[2], v[1], v[0]);

        __m128 q_vec =
            _mm_loadu_ps(q.data());

        __m128 q_w =
            _mm_set1_ps(q[3]);

        auto cross_f = [](__m128 a, __m128 b) {

            return _mm_sub_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(a, a, 0xC9),
                    _mm_shuffle_ps(b, b, 0xD2)
                ),
                _mm_mul_ps(
                    _mm_shuffle_ps(a, a, 0xD2),
                    _mm_shuffle_ps(b, b, 0xC9)
                )
            );
        };

        __m128 t =
            _mm_mul_ps(
                _mm_set1_ps(2.0f),
                cross_f(q_vec, v_simd)
            );

        __m128 res =
            _mm_add_ps(
                v_simd,
                _mm_add_ps(
                    _mm_mul_ps(q_w, t),
                    cross_f(q_vec, t)
                )
            );

        AlignedArray<T, 3> out;

        _mm_storeu_ps(out.data(), res);

        return out;
    }

    T tx = T{2} * (q[1]*v[2] - q[2]*v[1]);
    T ty = T{2} * (q[2]*v[0] - q[0]*v[2]);
    T tz = T{2} * (q[0]*v[1] - q[1]*v[0]);

    AlignedArray<T, 3> out;

    out[0] = v[0] + q[3]*tx + (q[1]*tz - q[2]*ty);
    out[1] = v[1] + q[3]*ty + (q[2]*tx - q[0]*tz);
    out[2] = v[2] + q[3]*tz + (q[0]*ty - q[1]*tx);

    return out;
}

/* ================= OPERADORES IN-PLACE ================= */

template <Scalar T, std::size_t N>
void operator+=(LinearStorage<T, N>& a,
                const LinearStorage<T, N>& b) {

    simd::apply_op<simd::Add>(
        a.data(),
        a.data(),
        b.data(),
        getSize<T, N>(a)
    );
}

template <Scalar T, std::size_t N>
void operator-=(LinearStorage<T, N>& a,
                const LinearStorage<T, N>& b) {

    simd::apply_op<simd::Sub>(
        a.data(),
        a.data(),
        b.data(),
        getSize<T, N>(a)
    );
}

/* ================= OPERADORES ================= */

template <Scalar T, std::size_t N>
auto operator+(const LinearStorage<T, N>& a,
               const LinearStorage<T, N>& b) {

    auto out = makeSimilar<T, N>(a);

    simd::apply_op<simd::Add>(
        out.data(),
        a.data(),
        b.data(),
        getSize<T, N>(a)
    );

    return out;
}

template <Scalar T, std::size_t N>
auto operator-(const LinearStorage<T, N>& a,
               const LinearStorage<T, N>& b) {

    auto out = makeSimilar<T, N>(a);

    simd::apply_op<simd::Sub>(
        out.data(),
        a.data(),
        b.data(),
        getSize<T, N>(a)
    );

    return out;
}

template <Scalar T, std::size_t N>
auto operator*(const LinearStorage<T, N>& a,
               const LinearStorage<T, N>& b) {

    auto out = makeSimilar<T, N>(a);

    simd::apply_op<simd::Mul>(
        out.data(),
        a.data(),
        b.data(),
        getSize<T, N>(a)
    );

    return out;
}

template <Scalar T, std::size_t N>
auto operator*(const LinearStorage<T, N>& v,
               T s) {

    return mul<T, N>(v, s);
}

template <Scalar T, std::size_t N>
auto operator*(T s,
               const LinearStorage<T, N>& v) {

    return mul<T, N>(v, s);
}

} // namespace geometry