#pragma once

#include "error.hpp"

#include <array>
#include <cmath>
#include <initializer_list>
#include <expected>
#include <stdexcept>
#include <type_traits>

namespace geometry {

template <typename T, std::size_t N, T EPS = T{}>
class VecN
{
    static_assert(N > 0, "VecN dimension must be greater than zero");
    static_assert(std::is_arithmetic_v<T>, "VecN requires arithmetic type");

private:
    std::array<T, N> m_data{};

    static constexpr T get_epsilon() noexcept
    {
        if constexpr (EPS > T{})
            return EPS;
        else if constexpr (std::is_floating_point_v<T>)
            return static_cast<T>(1e-8);
        else
            return T{};
    }

public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::array<T, N>::iterator;
    using const_iterator = typename std::array<T, N>::const_iterator;

    constexpr VecN() = default;

    explicit constexpr VecN(T value) noexcept
    {
        m_data.fill(value);
    }

    constexpr VecN(std::initializer_list<T> values)
    {
        auto it = values.begin();
        for (std::size_t i = 0; i < N && it != values.end(); ++i, ++it)
            m_data[i] = *it;
    }

    constexpr reference operator[](size_type i) noexcept
    { 
        return m_data[i]; 
    }
    
    constexpr const_reference operator[](size_type i) const noexcept
    { 
        return m_data[i]; 
    }

    constexpr iterator begin() noexcept { return m_data.begin(); }
    constexpr const_iterator begin() const noexcept { return m_data.begin(); }
    constexpr const_iterator cbegin() const noexcept { return m_data.cbegin(); }
    
    constexpr iterator end() noexcept { return m_data.end(); }
    constexpr const_iterator end() const noexcept { return m_data.end(); }
    constexpr const_iterator cend() const noexcept { return m_data.cend(); }

    constexpr size_type size() const noexcept { return N; }

    constexpr VecN operator+(const VecN& rhs) const noexcept
    {
        VecN result;
        for (size_type i = 0; i < N; ++i)
            result[i] = m_data[i] + rhs[i];
        return result;
    }

    constexpr VecN operator-(const VecN& rhs) const noexcept
    {
        VecN result;
        for (size_type i = 0; i < N; ++i)
            result[i] = m_data[i] - rhs[i];
        return result;
    }

    constexpr VecN operator-() const noexcept
    {
        VecN result;
        for (size_type i = 0; i < N; ++i)
            result[i] = -m_data[i];
        return result;
    }

    constexpr VecN operator*(T scalar) const noexcept
    {
        VecN result;
        for (size_type i = 0; i < N; ++i)
            result[i] = m_data[i] * scalar;
        return result;
    }

    constexpr VecN operator/(T scalar) const
    {
        if constexpr (!std::is_floating_point_v<T>)
        {
            if (scalar == T{})
                throw std::domain_error("Division by zero");
        }
        
        VecN result;
        for (size_type i = 0; i < N; ++i)
            result[i] = m_data[i] / scalar;
        return result;
    }

    constexpr VecN& operator+=(const VecN& rhs) noexcept
    {
        for (size_type i = 0; i < N; ++i)
            m_data[i] += rhs[i];
        return *this;
    }

    constexpr VecN& operator-=(const VecN& rhs) noexcept
    {
        for (size_type i = 0; i < N; ++i)
            m_data[i] -= rhs[i];
        return *this;
    }

    constexpr VecN& operator*=(T scalar) noexcept
    {
        for (size_type i = 0; i < N; ++i)
            m_data[i] *= scalar;
        return *this;
    }

    constexpr VecN& operator/=(T scalar)
    {
        if constexpr (!std::is_floating_point_v<T>)
        {
            if (scalar == T{})
                throw std::domain_error("Division by zero");
        }
        
        for (size_type i = 0; i < N; ++i)
            m_data[i] /= scalar;
        return *this;
    }

    constexpr T dot(const VecN& rhs) const noexcept
    {
        T sum{};
        for (size_type i = 0; i < N; ++i)
            sum += m_data[i] * rhs[i];
        return sum;
    }

    constexpr T length_squared() const noexcept
    {
        return dot(*this);
    }

    T length() const
    {
        if constexpr (std::is_floating_point_v<T>)
            return std::sqrt(length_squared());
        else
            return std::sqrt(static_cast<double>(length_squared()));
    }

    std::expected<VecN, Error> normalized() const
    {
        constexpr T eps = get_epsilon();
        T len_sq = length_squared();
        
        if (len_sq <= eps * eps)
            return std::unexpected(Error::make("Vector is zero"));
        
        if constexpr (std::is_floating_point_v<T>)
            return *this / std::sqrt(len_sq);
        else
            return *this / static_cast<T>(std::sqrt(static_cast<double>(len_sq)));
    }

    std::expected<T, Error> pseudoangle() const
    {
        static_assert(N == 2, "pseudoangle is only defined for 2D vectors");
        
        constexpr T eps = get_epsilon();
        
        const T x = m_data[0];
        const T y = m_data[1];
        
        // Check for zero vector
        if (std::abs(x) <= eps && std::abs(y) <= eps)
            return std::expected<T, Error>(std::unexpected(Error::make("Vector is zero")));
        
        const T ax = std::abs(x);
        const T ay = std::abs(y);
        const T sum = ax + ay;
        
        // Map angle to continuous range [0, 8)
        // using the formula that divides the plane into 8 octants
        if (x >= 0)
        {
            // Right half-plane
            return (y >= 0) ? ay / sum : static_cast<T>(8) - ay / sum;
        }
        else
        {
            // Left half-plane
            return (y >= 0) ? static_cast<T>(4) - ax / sum 
                            : static_cast<T>(4) + ay / sum;
        }
    }
    
    bool pseudoangle_less(const VecN<T, 2, EPS>& other) const
    {
        auto pa = this->pseudoangle();
        auto pb = other.pseudoangle();
        
        // Vectors with pseudoangle value are ordered before zero vectors
        if (!pa.has_value()) return pb.has_value();
        if (!pb.has_value()) return false;
        
        return pa.value() < pb.value();
    }
};

template <typename T, std::size_t N, T EPS>
constexpr bool operator==(const VecN<T, N, EPS>& lhs, const VecN<T, N, EPS>& rhs) noexcept
{
    for (std::size_t i = 0; i < N; ++i)
        if (lhs[i] != rhs[i]) return false;
    return true;
}

template <typename T, std::size_t N, T EPS>
constexpr bool operator!=(const VecN<T, N, EPS>& lhs, const VecN<T, N, EPS>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T, T EPS>
constexpr T cross(const VecN<T, 2, EPS>& a, const VecN<T, 2, EPS>& b) noexcept
{
    return a[0] * b[1] - a[1] * b[0];
}

template <typename T, T EPS>
constexpr VecN<T, 3, EPS> cross(const VecN<T, 3, EPS>& a, const VecN<T, 3, EPS>& b) noexcept
{
    return VecN<T, 3, EPS>{
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

template <typename T, T EPS>
constexpr VecN<T, 4, EPS> cross(
    const VecN<T, 4, EPS>& a,
    const VecN<T, 4, EPS>& b,
    const VecN<T, 4, EPS>& c) noexcept
{
    return VecN<T, 4, EPS>{
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

template <typename T, std::size_t N, T EPS>
constexpr VecN<T, N, EPS> operator*(T scalar, const VecN<T, N, EPS>& v) noexcept
{
    return v * scalar;
}

template <typename T, T EPS>
std::expected<T, Error> pseudoangle(const VecN<T, 2, EPS>& v) noexcept
{
    return v.pseudoangle();
}

template <typename T, T EPS>
bool pseudoangle_less(const VecN<T, 2, EPS>& a, const VecN<T, 2, EPS>& b) noexcept
{
    return a.pseudoangle_less(b);
}

} // namespace geometry