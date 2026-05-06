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
#include "vector.hpp"

namespace geometry {

/* ================= POINT ================= */

template <Arithmetic T, std::size_t N>
class Point {
public:
    enum class Error {
        SizeMismatch,
        DivisionByZero,
        InvalidDimension
    };

    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;

    VectorOrArray<T, N> data;

    /* ================= CONSTRUCTORS ================= */

    Point() = default;

    Point(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N)
                throw std::invalid_argument("Dimensão incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    explicit Point(std::size_t size) {
        static_assert(N == 0, "Apenas pontos dinâmicos podem especificar tamanho");
        data.resize(size);
    }

    explicit Point(const VectorType& vec) {
        if constexpr (N == 0) {
            data.assign(vec.data.begin(), vec.data.end());
        } else {
            static_assert(N != 0, "Dimensão fixa requer mesmo tamanho");
            std::copy(vec.data.begin(), vec.data.end(), data.begin());
        }
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

    T& operator[](std::size_t i) {
        if constexpr (N == 0) {
            if (i >= data.size())
                throw std::out_of_range("Point index out of range");
        }
        return data[i];
    }

    const T& operator[](std::size_t i) const {
        if constexpr (N == 0) {
            if (i >= data.size())
                throw std::out_of_range("Point index out of range");
        }
        return data[i];
    }

    /* ================= POINTER ACCESS ================= */

    T* data_ptr() noexcept {
        return data.data();
    }

    const T* data_ptr() const noexcept {
        return data.data();
    }

    /* ================= CONVERSIONS ================= */

    VectorType to_vector() const {
        VectorType result;
        if constexpr (N == 0) {
            result = VectorType(size());
            for (std::size_t i = 0; i < size(); ++i)
                result[i] = data[i];
        } else {
            for (std::size_t i = 0; i < N; ++i)
                result[i] = data[i];
        }
        return result;
    }

    /* ================= DISTANCE OPERATIONS ================= */

    T distance_to(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        T sum_sq{};
        for (std::size_t i = 0; i < size(); ++i) {
            T diff = data[i] - other[i];
            sum_sq += diff * diff;
        }
        return std::sqrt(sum_sq);
    }

    T squared_distance_to(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        T sum_sq{};
        for (std::size_t i = 0; i < size(); ++i) {
            T diff = data[i] - other[i];
            sum_sq += diff * diff;
        }
        return sum_sq;
    }

    /* ================= VECTOR OPERATIONS ================= */

    Point operator+(const VectorType& vec) const {
        if constexpr (N == 0) {
            if (size() != vec.size())
                throw std::runtime_error("Size mismatch");
        }
        
        Point result = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            result[i] = data[i] + vec[i];
        return result;
    }

    Point operator-(const VectorType& vec) const {
        if constexpr (N == 0) {
            if (size() != vec.size())
                throw std::runtime_error("Size mismatch");
        }
        
        Point result = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            result[i] = data[i] - vec[i];
        return result;
    }

    VectorType operator-(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        VectorType result = make_vector_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            result[i] = data[i] - other[i];
        return result;
    }

    /* ================= INTERPOLATION ================= */

    Point lerp(const Point& other, T t) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        Point result = make_result(*this);
        for (std::size_t i = 0; i < size(); ++i)
            result[i] = data[i] + (other[i] - data[i]) * t;
        return result;
    }

    /* ================= COMPARISONS ================= */

    bool operator==(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                return false;
        }
        
        for (std::size_t i = 0; i < size(); ++i) {
            if (data[i] != other[i])
                return false;
        }
        return true;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "(";
        for (std::size_t i = 0; i < p.size(); ++i) {
            os << p[i];
            if (i + 1 < p.size()) os << ", ";
        }
        os << ")";
        return os;
    }

    /* ================= UTILITY ================= */

    Point midpoint(const Point& other) const {
        return lerp(other, static_cast<T>(0.5));
    }

    T manhattan_distance(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        T sum{};
        for (std::size_t i = 0; i < size(); ++i)
            sum += std::abs(data[i] - other[i]);
        return sum;
    }

    T chebyshev_distance(const Point& other) const {
        if constexpr (N == 0) {
            if (size() != other.size())
                throw std::runtime_error("Size mismatch");
        }
        
        T max_diff{};
        for (std::size_t i = 0; i < size(); ++i) {
            T diff = std::abs(data[i] - other[i]);
            if (diff > max_diff)
                max_diff = diff;
        }
        return max_diff;
    }

private:
    static Point make_result(const Point& ref) {
        if constexpr (N == 0) return Point(ref.size());
        else return Point{};
    }

    static VectorType make_vector_result(const Point& ref) {
        if constexpr (N == 0) return VectorType(ref.size());
        else return VectorType{};
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T>
using Point2 = Point<T, 2>;

template <Arithmetic T>
using Point3 = Point<T, 3>;

template <Arithmetic T>
using Point4 = Point<T, 4>;

using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;

} // namespace geometry