#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include "arithmetic.hpp"
#include "point.hpp"
#include "vector.hpp"
#include "segment.hpp"

namespace geometry {

/**
 * @brief Classe Ray definida por uma origem (Ponto) e uma direção (Vetor).
 */
template <Arithmetic T, std::size_t N>
class Ray {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;
    using SegmentType = Segment<T, N>;

private:
    PointType origin;
    VectorType direction_vec;

public:
    /* ================= CONSTRUTORES ================= */

    Ray() = default;

    Ray(const PointType& orig, const VectorType& dir)
        : origin(orig), direction_vec(dir) {
        
        if constexpr (N == 0) {
            if (orig.size() != dir.size())
                throw std::runtime_error("Size mismatch entre origem e direção");
        }
        
        if (direction_vec.dot(direction_vec) <= T{0})
            throw std::runtime_error("A direção do raio não pode ser um vetor nulo");
    }

    // Raio passando por dois pontos
    Ray(const PointType& orig, const PointType& target)
        : origin(orig), direction_vec(target - orig) {
        if (direction_vec.dot(direction_vec) <= T{0})
            throw std::runtime_error("Os pontos de origem e destino devem ser distintos");
    }

    /* ================= ACESSO ================= */

    const PointType& get_origin() const noexcept { return origin; }
    const VectorType& get_direction() const noexcept { return direction_vec; }
    
    void set_origin(const PointType& orig) { origin = orig; }
    void set_direction(const VectorType& dir) { 
        if (dir.dot(dir) <= T{0})
            throw std::runtime_error("A direção do raio não pode ser um vetor nulo");
        direction_vec = dir;
    }

    /* ================= OPERAÇÕES DE PONTO ================= */

    // P(t) = O + t * D
    PointType at(T t) const {
        if (t < T{0}) throw std::runtime_error("O parâmetro t do raio deve ser >= 0");
        return origin + (direction_vec * t);
    }

    PointType operator()(T t) const { return at(t); }

    /* ================= NORMALIZAÇÃO ================= */

    Ray normalized() const {
        return Ray(origin, direction_vec.normalized());
    }

    /* ================= GEOMETRIA ================= */

    T squared_distance_to_point(const PointType& p) const {
        VectorType to_p = p - origin;
        T t = to_p.dot(direction_vec) / direction_vec.dot(direction_vec);
        
        if (t < T{0}) {
            // O ponto mais próximo é a origem (atrás do raio)
            return to_p.dot(to_p);
        }
        
        PointType closest = origin + (direction_vec * t);
        return p.squared_distance_to(closest);
    }

    T distance_to_point(const PointType& p) const {
        return std::sqrt(squared_distance_to_point(p));
    }

    PointType closest_point_to(const PointType& p) const {
        VectorType to_p = p - origin;
        T projection = to_p.dot(direction_vec);
        
        if (projection <= T{0}) return origin;
        
        T t = projection / direction_vec.dot(direction_vec);
        return origin + (direction_vec * t);
    }

    /* ================= INTERSECÇÃO ================= */

    std::optional<PointType> intersection(const SegmentType& seg, T eps = static_cast<T>(1e-8)) const {
        if constexpr (N == 2) {
            // Reutiliza a lógica de cross product 2D do arithmetic.hpp
            Vector2<T> v1 = direction_vec;
            Vector2<T> v2 = seg.end() - seg.start();
            Vector2<T> v3 = seg.start() - origin;

            T det = v1[0] * v2[1] - v1[1] * v2[0];
            if (std::abs(det) < eps) return std::nullopt; // Paralelos

            T t = (v3[0] * v2[1] - v3[1] * v2[0]) / det;
            T u = (v3[0] * v1[1] - v3[1] * v1[0]) / det;

            if (t >= -eps && u >= -eps && u <= T{1} + eps) {
                return at(std::max(T{0}, t));
            }
        } 
        else if constexpr (N == 3) {
            // Lógica de intersecção 3D usando cross product do Vector
            Vector3<T> seg_dir = seg.end() - seg.start();
            Vector3<T> p_diff = seg.start() - origin;
            Vector3<T> cp = direction_vec.cross(seg_dir);
            T det = cp.dot(cp);

            if (det < eps) return std::nullopt;

            T t = p_diff.cross(seg_dir).dot(cp) / det;
            T u = p_diff.cross(direction_vec).dot(cp) / det;

            if (t >= -eps && u >= -eps && u <= T{1} + eps) {
                return at(std::max(T{0}, t));
            }
        }
        return std::nullopt;
    }

    /* ================= REFLEXÃO ================= */

    Ray reflect(const PointType& contact_point, const VectorType& normal) const {
        // R = I - 2(I.N)N
        VectorType incident = direction_vec.normalized();
        T dot = incident.dot(normal);
        VectorType reflected = incident - (normal * (static_cast<T>(2) * dot));
        return Ray(contact_point, reflected);
    }

    /* ================= BOOLEANOS ================= */

    bool contains_point(const PointType& p, T eps = static_cast<T>(1e-8)) const {
        VectorType to_p = p - origin;
        // Verifica se é colinear via cross product (norma deve ser ~0)
        if constexpr (N == 2) {
            T cp = to_p[0] * direction_vec[1] - to_p[1] * direction_vec[0];
            if (std::abs(cp) > eps) return false;
        } else if constexpr (N == 3) {
            if (to_p.cross(direction_vec).dot(to_p.cross(direction_vec)) > eps) return false;
        }
        
        return to_p.dot(direction_vec) >= -eps;
    }

    bool operator==(const Ray& other) const {
        // Raios são iguais se têm mesma origem e direções normalizadas idênticas
        if (origin != other.origin) return false;
        return direction_vec.normalized() == other.direction_vec.normalized();
    }

    friend std::ostream& operator<<(std::ostream& os, const Ray& r) {
        os << "Ray(O: " << r.origin << ", D: " << r.direction_vec << ")";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T> using Ray2 = Ray<T, 2>;
template <Arithmetic T> using Ray3 = Ray<T, 3>;
using Ray2f = Ray2<float>;
using Ray3f = Ray3<float>;

} // namespace geometry