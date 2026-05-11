#pragma once

#include "arithmetic.hpp"
#include "vector.hpp"
#include <iostream>
#include <span>
#include <initializer_list>
#include <algorithm>
#include <cmath>

namespace geometry {

/**
 * @brief Classe Point focada em localização, usando arithmetic.hpp para cálculos.
 */
template <Arithmetic T, std::size_t N>
class Point {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;
    using Storage = ArithmeticVector<T, N>;

    Storage data;

    /* ================= CONSTRUTORES ================= */

    Point() {
        if constexpr (N == 0) {
            // Dinâmico: inicia vazio
        } else {
            data.fill(T{0});
        }
    }

    Point(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N) throw std::invalid_argument("Dimensao incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    explicit Point(std::size_t size) requires (N == 0)
        : data(size, T{0}) {}

    // Converte Vector para Point
    explicit Point(const VectorType& vec) {
        data = vec.data; 
    }

    /* ================= ACESSO ================= */

    std::size_t size() const noexcept { return geometry::getSize<T, N>(data); }
    T* data_ptr() noexcept { return data.data(); }
    const T* data_ptr() const noexcept { return data.data(); }
    
    T& operator[](std::size_t i) { return data[i]; }
    const T& operator[](std::size_t i) const { return data[i]; }

    /* ================= OPERAÇÕES ALGEBRICAS ================= */

    // Ponto + Vetor = Ponto (Deslocamento)
    Point operator+(const VectorType& vec) const {
        Point res;
        if constexpr (N == 0) res.data.resize(size());
        // Uso do 'template' para resolver erro de ambiguidade do compilador
        res.data = geometry::template operator+<T, N>(this->data, vec.data);
        return res;
    }

    // Ponto - Vetor = Ponto
    Point operator-(const VectorType& vec) const {
        Point res;
        if constexpr (N == 0) res.data.resize(size());
        res.data = geometry::template operator-<T, N>(this->data, vec.data);
        return res;
    }

    // Ponto - Ponto = Vetor (Diferença de posição)
    VectorType operator-(const Point& other) const {
        VectorType res;
        if constexpr (N == 0) res.data.resize(size());
        res.data = geometry::template operator-<T, N>(this->data, other.data);
        return res;
    }

    /* ================= DISTÂNCIAS (Otimizadas) ================= */

    T squared_distance_to(const Point& other) const {
        // Aproveita a implementação SIMD de subtração e produto escalar
        VectorType diff = *this - other;
        return diff.dot(diff);
    }

    T distance_to(const Point& other) const {
        return std::sqrt(squared_distance_to(other));
    }

    T manhattan_distance(const Point& other) const {
        T sum{};
        for (std::size_t i = 0; i < size(); ++i)
            sum += std::abs(data[i] - other.data[i]);
        return sum;
    }

    /* ================= INTERPOLAÇÃO ================= */

    Point lerp(const Point& other, T t) const {
        Point res;
        if constexpr (N == 0) res.data.resize(size());
        
        // res = P0 + (P1 - P0) * t
        // Aproveita os operadores sobrecarregados que já usam SIMD
        VectorType diff = other - *this;
        res = *this + (diff * t);
        return res;
    }

    Point midpoint(const Point& other) const {
        return lerp(other, static_cast<T>(0.5));
    }

    /* ================= COMPARAÇÃO E I/O ================= */

    bool operator==(const Point& other) const { return data == other.data; }
    bool operator!=(const Point& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "P[";
        for (std::size_t i = 0; i < p.size(); ++i) {
            os << p.data[i] << (i == p.size() - 1 ? "" : ", ");
        }
        os << "]";
        return os;
    }

    VectorType to_vector() const {
        VectorType v;
        v.data = this->data;
        return v;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T> using Point2 = Point<T, 2>;
template <Arithmetic T> using Point3 = Point<T, 3>;

using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;

} // namespace geometry