#pragma once

#include <array>
#include <cmath>
#include <initializer_list>
#include <optional>
#include <type_traits>

template <typename T, unsigned int N, T EPS = T{}>
struct VecN
{
    static_assert(N > 0, "VecN dimension must be greater than zero");

    std::array<T, N> data{};

    /* =========================
       Construtores
       ========================= */

    constexpr VecN() = default;

    explicit constexpr VecN(T value)
    {
        data.fill(value);
    }

    constexpr VecN(std::initializer_list<T> values)
    {
        std::size_t i = 0;
        for (T v : values)
        {
            if (i < N) data[i++] = v;
        }
    }

    /* =========================
       Acesso
       ========================= */

    constexpr T& operator[](unsigned int i) { return data[i]; }
    constexpr const T& operator[](unsigned int i) const { return data[i]; }

    /* =========================
       Operações básicas
       ========================= */

    constexpr VecN operator+(const VecN& rhs) const
    {
        VecN r;
        for (unsigned int i = 0; i < N; ++i)
            r[i] = data[i] + rhs[i];
        return r;
    }

    constexpr VecN operator-(const VecN& rhs) const
    {
        VecN r;
        for (unsigned int i = 0; i < N; ++i)
            r[i] = data[i] - rhs[i];
        return r;
    }

    constexpr VecN operator*(T scalar) const
    {
        VecN r;
        for (unsigned int i = 0; i < N; ++i)
            r[i] = data[i] * scalar;
        return r;
    }

    constexpr VecN operator/(T scalar) const
    {
        VecN r;
        for (unsigned int i = 0; i < N; ++i)
            r[i] = data[i] / scalar;
        return r;
    }

    /* =========================
       Álgebra vetorial
       ========================= */

    constexpr T dot(const VecN& rhs) const
    {
        T sum{};
        for (unsigned int i = 0; i < N; ++i)
            sum += data[i] * rhs[i];
        return sum;
    }

    constexpr T lengthSquared() const
    {
        return dot(*this);
    }

    T length() const
    {
        T lenSq = lengthSquared();
        return std::sqrt(lenSq);
    }

    std::optional<VecN> normalized() const
    {
        constexpr T eps =
            (EPS > T{}) ? EPS :
            (std::is_floating_point_v<T> ? static_cast<T>(1e-8) : T{});

        T lenSq = lengthSquared();
        if (lenSq <= eps * eps)
            return std::nullopt;

        return (*this) / std::sqrt(lenSq);
    }
};

/* =========================
   Cross product
   ========================= */

// 2D → retorna Z do cross 3D
template <typename T, T EPS>
constexpr T cross(const VecN<T, 2, EPS>& a, const VecN<T, 2, EPS>& b)
{
    return a[0] * b[1] - a[1] * b[0];
}

// 3D → vetor
template <typename T, T EPS>
constexpr VecN<T, 3, EPS> cross(const VecN<T, 3, EPS>& a, const VecN<T, 3, EPS>& b)
{
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

// 4D → produto vetorial de 3 vetores
template <typename T, T EPS>
constexpr VecN<T, 4, EPS> cross(
    const VecN<T, 4, EPS>& a,
    const VecN<T, 4, EPS>& b,
    const VecN<T, 4, EPS>& c)
{
    return {
        + (a[1]*(b[2]*c[3] - b[3]*c[2])
         - a[2]*(b[1]*c[3] - b[3]*c[1])
         + a[3]*(b[1]*c[2] - b[2]*c[1])),

        - (a[0]*(b[2]*c[3] - b[3]*c[2])
         - a[2]*(b[0]*c[3] - b[3]*c[0])
         + a[3]*(b[0]*c[2] - b[2]*c[0])),

        + (a[0]*(b[1]*c[3] - b[3]*c[1])
         - a[1]*(b[0]*c[3] - b[3]*c[0])
         + a[3]*(b[0]*c[1] - b[1]*c[0])),

        - (a[0]*(b[1]*c[2] - b[2]*c[1])
         - a[1]*(b[0]*c[2] - b[2]*c[0])
         + a[2]*(b[0]*c[1] - b[1]*c[0]))
    };
}

/* =========================
   Escalar * Vetor
   ========================= */

template <typename T, unsigned int N, T EPS>
constexpr VecN<T, N, EPS> operator*(T scalar, const VecN<T, N, EPS>& v)
{
    return v * scalar;
}