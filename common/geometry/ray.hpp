#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include "point.hpp"
#include "vector.hpp"
#include "segment.hpp"

namespace geometry {

/* ================= RAY ================= */

template <Arithmetic T, std::size_t N>
class Ray {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;
    using SegmentType = Segment<T, N>;
    using RayType = Ray<T, N>;

private:
    PointType origin;
    VectorType direction_vec;

public:
    /* ================= CONSTRUCTORS ================= */

    Ray() = default;

    Ray(const PointType& orig, const VectorType& dir)
        : origin(orig), direction_vec(dir) {
        if constexpr (N == 0) {
            if (orig.size() != dir.size())
                throw std::runtime_error("Size mismatch");
        }
        
        // Verifica se a direção não é zero
        T len2 = dir.dot(dir);
        if (len2 <= static_cast<T>(0))
            throw std::runtime_error("Ray direction cannot be zero");
    }

    Ray(const PointType& orig, const PointType& dir_point)
        : origin(orig), direction_vec(dir_point - orig) {
        T len2 = direction_vec.dot(direction_vec);
        if (len2 <= static_cast<T>(0))
            throw std::runtime_error("Ray direction cannot be zero");
    }

    /* ================= ACCESSORS ================= */

    const PointType& get_origin() const noexcept { return origin; }
    const VectorType& get_direction() const noexcept { return direction_vec; }
    
    void set_origin(const PointType& orig) { origin = orig; }
    void set_direction(const VectorType& dir) { 
        T len2 = dir.dot(dir);
        if (len2 <= static_cast<T>(0))
            throw std::runtime_error("Ray direction cannot be zero");
        direction_vec = dir;
    }

    /* ================= NORMALIZATION ================= */

    Ray normalized(T eps = static_cast<T>(1e-8)) const {
        T len = std::sqrt(direction_vec.dot(direction_vec));
        if (len <= eps)
            throw std::runtime_error("Cannot normalize zero direction");
        return Ray(origin, direction_vec / len);
    }

    VectorType normalized_direction(T eps = static_cast<T>(1e-8)) const {
        T len = std::sqrt(direction_vec.dot(direction_vec));
        if (len <= eps)
            throw std::runtime_error("Cannot normalize zero direction");
        return direction_vec / len;
    }

    /* ================= POINT EVALUATION ================= */

    PointType at(T t) const {
        // t >= 0
        if (t < static_cast<T>(0))
            throw std::runtime_error("Ray parameter must be >= 0");
        return origin + direction_vec * t;
    }

    PointType operator()(T t) const {
        return at(t);
    }

    /* ================= DISTANCE TO POINT ================= */

    T distance_to_point(const PointType& point) const {
        return std::sqrt(squared_distance_to_point(point));
    }

    T squared_distance_to_point(const PointType& point) const {
        VectorType to_point = point - origin;
        T dot = to_point.dot(direction_vec);
        T dir_len2 = direction_vec.dot(direction_vec);
        
        if (dot <= static_cast<T>(0)) {
            // Ponto está atrás da origem
            return to_point.dot(to_point);
        }
        
        // Projeção no raio
        T t = dot / dir_len2;
        PointType closest = origin + direction_vec * t;
        return point.squared_distance_to(closest);
    }

    PointType closest_point_to(const PointType& point) const {
        VectorType to_point = point - origin;
        T dot = to_point.dot(direction_vec);
        
        if (dot <= static_cast<T>(0)) {
            return origin;
        }
        
        T t = dot / direction_vec.dot(direction_vec);
        return origin + direction_vec * t;
    }

    /* ================= INTERSECTION ================= */

    std::optional<PointType> intersection(const SegmentType& segment, T eps = static_cast<T>(1e-8)) const {
        if constexpr (N == 2) {
            // Implementação 2D
            const PointType& a = origin;
            const PointType& b = segment.start();
            const PointType& c = segment.end();
            
            Vector<T, 2> dir = direction_vec;
            Vector<T, 2> seg_dir = c - b;
            Vector<T, 2> a_to_b = b - a;
            
            T cross_dir_seg = dir[0] * seg_dir[1] - dir[1] * seg_dir[0];
            
            if (std::abs(cross_dir_seg) < eps) {
                // Paralelo
                return std::nullopt;
            }
            
            T t = (a_to_b[0] * seg_dir[1] - a_to_b[1] * seg_dir[0]) / cross_dir_seg;
            T u = (a_to_b[0] * dir[1] - a_to_b[1] * dir[0]) / cross_dir_seg;
            
            if (t >= -eps && u >= -eps && u <= static_cast<T>(1) + eps) {
                if (t < 0) return std::nullopt;
                return a + dir * t;
            }
            
            return std::nullopt;
        } else if constexpr (N == 3) {
            // Implementação 3D simplificada
            const PointType& a = origin;
            const PointType& b = segment.start();
            const PointType& c = segment.end();
            
            Vector<T, 3> dir = direction_vec;
            Vector<T, 3> seg_dir = c - b;
            Vector<T, 3> a_to_b = b - a;
            
            Vector<T, 3> cross = dir.cross(seg_dir);
            T denom = cross.dot(cross);
            
            if (denom < eps) {
                return std::nullopt; // Paralelo
            }
            
            // Resolve sistema linear (simplificado)
            Vector<T, 3> a_to_b_cross_seg = a_to_b.cross(seg_dir);
            T t = a_to_b_cross_seg.dot(cross) / denom;
            
            if (t < -eps) return std::nullopt;
            
            Vector<T, 3> a_to_b_cross_dir = a_to_b.cross(dir);
            T u = a_to_b_cross_dir.dot(cross) / denom;
            
            if (u >= -eps && u <= static_cast<T>(1) + eps) {
                return a + dir * t;
            }
            
            return std::nullopt;
        } else {
            static_assert(N == 2 || N == 3, "Intersection only implemented for 2D/3D rays");
            return std::nullopt;
        }
    }

    std::optional<PointType> intersection(const Ray& other, T eps = static_cast<T>(1e-8)) const {
        if constexpr (N == 2) {
            // Intersecção de dois raios em 2D
            const PointType& a = origin;
            const PointType& b = other.origin;
            
            Vector<T, 2> dir = direction_vec;
            Vector<T, 2> other_dir = other.direction_vec;
            Vector<T, 2> a_to_b = b - a;
            
            T cross_dir_other = dir[0] * other_dir[1] - dir[1] * other_dir[0];
            
            if (std::abs(cross_dir_other) < eps) {
                return std::nullopt; // Paralelos
            }
            
            T t = (a_to_b[0] * other_dir[1] - a_to_b[1] * other_dir[0]) / cross_dir_other;
            T u = (a_to_b[0] * dir[1] - a_to_b[1] * dir[0]) / cross_dir_other;
            
            if (t >= -eps && u >= -eps) {
                return a + dir * t;
            }
            
            return std::nullopt;
        } else {
            static_assert(N == 2, "Ray-ray intersection only implemented for 2D");
            return std::nullopt;
        }
    }

    /* ================= REFLECTION ================= */

    Ray reflect(const PointType& point, const VectorType& normal) const {
        // Reflete o raio em um ponto com uma normal dada
        VectorType incident = (point - origin).normalized();
        T dot = incident.dot(normal);
        VectorType reflected = incident - normal * (static_cast<T>(2) * dot);
        return Ray(point, reflected);
    }

    /* ================= CONTAINMENT ================= */

    bool contains_point(const PointType& point, T eps = static_cast<T>(1e-8)) const {
        VectorType to_point = point - origin;
        
        // Verifica se são colineares
        if constexpr (N == 2) {
            T cross = to_point[0] * direction_vec[1] - to_point[1] * direction_vec[0];
            if (std::abs(cross) > eps)
                return false;
        } else if constexpr (N == 3) {
            VectorType cross = to_point.cross(direction_vec);
            if (std::abs(cross[0]) > eps || std::abs(cross[1]) > eps || std::abs(cross[2]) > eps)
                return false;
        }
        
        // Verifica se está na mesma direção
        T dot = to_point.dot(direction_vec);
        return dot >= -eps;
    }

    /* ================= COMPARISONS ================= */

    bool operator==(const Ray& other) const {
        if constexpr (N == 0) {
            if (origin.size() != other.origin.size())
                return false;
        }
        
        // Verifica se as origens são iguais e as direções são paralelas e mesma orientação
        if (origin != other.origin)
            return false;
        
        VectorType normalized_dir = normalized_direction();
        VectorType other_normalized_dir = other.normalized_direction();
        
        return normalized_dir == other_normalized_dir;
    }

    bool operator!=(const Ray& other) const {
        return !(*this == other);
    }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Ray& ray) {
        os << "Ray{origin: " << ray.origin << ", direction: " << ray.direction_vec << "}";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T>
using Ray2 = Ray<T, 2>;

template <Arithmetic T>
using Ray3 = Ray<T, 3>;

using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;
using Ray3f = Ray3<float>;
using Ray3d = Ray3<double>;

} // namespace geometry