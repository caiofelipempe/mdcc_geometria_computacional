#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include <algorithm>
#include "arithmetic.hpp"
#include "point.hpp"
#include "vector.hpp"

namespace geometry {

/**
 * @brief Classe Segment definida por dois pontos (p0 e p1).
 */
template <Arithmetic T, std::size_t N>
class Segment {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;

private:
    PointType p0;
    PointType p1;

public:
    /* ================= CONSTRUTORES ================= */

    Segment() = default;

    Segment(const PointType& start, const PointType& end)
        : p0(start), p1(end) {
        if constexpr (N == 0) {
            if (start.size() != end.size())
                throw std::runtime_error("Size mismatch entre pontos do segmento");
        }
    }

    Segment(std::initializer_list<PointType> points) {
        if (points.size() != 2)
            throw std::invalid_argument("Segment requer exatamente 2 pontos");
        auto it = points.begin();
        p0 = *it;
        p1 = *(++it);
    }

    /* ================= ACESSO ================= */

    const PointType& start() const noexcept { return p0; }
    const PointType& end() const noexcept { return p1; }
    
    void set_start(const PointType& point) { p0 = point; }
    void set_end(const PointType& point) { p1 = point; }

    /* ================= PROPRIEDADES ================= */

    VectorType direction() const { return p1 - p0; }

    T length() const { return p0.distance_to(p1); }

    T squared_length() const { return p0.squared_distance_to(p1); }

    PointType midpoint() const { return p0.midpoint(p1); }

    /* ================= AVALIAÇÃO E DISTÂNCIA ================= */

    // t=0 -> p0, t=1 -> p1
    PointType at(T t) const {
        return p0 + (direction() * t);
    }

    PointType closest_point_to(const PointType& p) const {
        VectorType dir = direction();
        T l2 = dir.dot(dir);
        if (l2 <= T{0}) return p0; // Segmento degenerado (ponto)

        // Projeção escalar t = [(p - p0) . dir] / |dir|^2
        T t = (p - p0).dot(dir) / l2;
        
        // Clamp t no intervalo [0, 1] para manter o ponto dentro do segmento
        t = std::max(T{0}, std::min(T{1}, t));
        
        return p0 + (dir * t);
    }

    T squared_distance_to_point(const PointType& p) const {
        return p.squared_distance_to(closest_point_to(p));
    }

    T distance_to_point(const PointType& p) const {
        return std::sqrt(squared_distance_to_point(p));
    }

    /* ================= INTERSECÇÃO (2D) ================= */

    /**
     * @brief Intersecção de segmentos em 2D usando o algoritmo de cross product.
     */
    std::optional<PointType> intersection(const Segment& other, T eps = static_cast<T>(1e-8)) const {
        if constexpr (N == 2) {
            const PointType& a = p0;
            const PointType& b = p1;
            const PointType& c = other.p0;
            const PointType& d = other.p1;

            VectorType r = b - a;
            VectorType s = d - c;
            
            // Cross product 2D: (r x s)
            T rxs = r[0] * s[1] - r[1] * s[0];
            
            if (std::abs(rxs) < eps) return std::nullopt; // Paralelos ou Colineares

            VectorType qp = c - a;
            // t = (q - p) x s / (r x s)
            T t = (qp[0] * s[1] - qp[1] * s[0]) / rxs;
            // u = (q - p) x r / (r x s)
            T u = (qp[0] * r[1] - qp[1] * r[0]) / rxs;

            if (t >= -eps && t <= T{1} + eps && u >= -eps && u <= T{1} + eps) {
                return at(std::clamp(t, T{0}, T{1}));
            }
        } else {
            // Para N=3, a intersecção exata é rara (segmentos precisam ser coplanares)
            // Geralmente usa-se a distância entre retas ou uma implementação específica.
            static_assert(N == 2, "Intersecção de segmentos implementada apenas para 2D");
        }
        return std::nullopt;
    }

    /* ================= RELACIONAMENTOS ================= */

    bool contains_point(const PointType& p, T eps = static_cast<T>(1e-8)) const {
        // Um ponto está no segmento se a distância até ele for quase zero
        return squared_distance_to_point(p) <= (eps * eps);
    }

    /* ================= COMPARISONS ================= */

    bool operator==(const Segment& other) const {
        // Segmentos são iguais mesmo se start/end estiverem invertidos
        return (p0 == other.p0 && p1 == other.p1) ||
               (p0 == other.p1 && p1 == other.p0);
    }

    bool operator!=(const Segment& other) const { return !(*this == other); }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Segment& seg) {
        os << "Seg[" << seg.p0 << " -> " << seg.p1 << "]";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T> using Segment2 = Segment<T, 2>;
template <Arithmetic T> using Segment3 = Segment<T, 3>;
using Segment2f = Segment2<float>;
using Segment3f = Segment3<float>;

} // namespace geometry