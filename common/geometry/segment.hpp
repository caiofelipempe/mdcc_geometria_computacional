#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include "point.hpp"
#include "vector.hpp"

namespace geometry {

/* ================= SEGMENT ================= */

template <Arithmetic T, std::size_t N>
class Segment {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;
    using SegmentType = Segment<T, N>;

private:
    PointType p0;
    PointType p1;

public:
    /* ================= CONSTRUCTORS ================= */

    Segment() = default;

    Segment(const PointType& start, const PointType& end)
        : p0(start), p1(end) {
        if constexpr (N == 0) {
            if (start.size() != end.size())
                throw std::runtime_error("Size mismatch");
        }
    }

    Segment(std::initializer_list<PointType> points) {
        if (points.size() != 2)
            throw std::invalid_argument("Segment requires exactly 2 points");
        auto it = points.begin();
        p0 = *it;
        p1 = *(++it);
    }

    /* ================= ACCESSORS ================= */

    const PointType& start() const noexcept { return p0; }
    const PointType& end() const noexcept { return p1; }
    
    void set_start(const PointType& point) { p0 = point; }
    void set_end(const PointType& point) { p1 = point; }

    /* ================= PROPERTIES ================= */

    VectorType direction() const {
        return p1 - p0;
    }

    VectorType normalized_direction(T eps = static_cast<T>(1e-8)) const {
        VectorType dir = direction();
        T len = std::sqrt(dir.dot(dir));
        if (len <= eps)
            throw std::runtime_error("Degenerate segment (zero length)");
        return dir / len;
    }

    T length() const {
        return p0.distance_to(p1);
    }

    T squared_length() const {
        return p0.squared_distance_to(p1);
    }

    PointType midpoint() const {
        return p0.midpoint(p1);
    }

    /* ================= POINT EVALUATION ================= */

    PointType at(T t) const {
        // t = 0 -> p0, t = 1 -> p1
        return p0 + direction() * t;
    }

    PointType closest_point_to(const PointType& point) const {
        VectorType dir = direction();
        VectorType to_point = point - p0;
        
        T t = dir.dot(to_point) / squared_length();
        
        if (t <= static_cast<T>(0))
            return p0;
        if (t >= static_cast<T>(1))
            return p1;
        
        return p0 + dir * t;
    }

    T distance_to_point(const PointType& point) const {
        return point.distance_to(closest_point_to(point));
    }

    T squared_distance_to_point(const PointType& point) const {
        return point.squared_distance_to(closest_point_to(point));
    }

    /* ================= INTERSECTION ================= */

    std::optional<PointType> intersection(const Segment& other, T eps = static_cast<T>(1e-8)) const {
        if constexpr (N == 2) {
            // Implementação para 2D usando determinantes
            const PointType& a = p0;
            const PointType& b = p1;
            const PointType& c = other.p0;
            const PointType& d = other.p1;

            Vector<T, 2> r = b - a;
            Vector<T, 2> s = d - c;
            
            T cross_r_s = r[0] * s[1] - r[1] * s[0];
            
            if (std::abs(cross_r_s) < eps) {
                // Segmentos são paralelos ou colineares
                return std::nullopt;
            }
            
            Vector<T, 2> qp = c - a;
            T t = (qp[0] * s[1] - qp[1] * s[0]) / cross_r_s;
            T u = (qp[0] * r[1] - qp[1] * r[0]) / cross_r_s;
            
            if (t >= -eps && t <= static_cast<T>(1) + eps && 
                u >= -eps && u <= static_cast<T>(1) + eps) {
                
                t = std::clamp(t, static_cast<T>(0), static_cast<T>(1));
                return a + r * t;
            }
            
            return std::nullopt;
        } else {
            // Para outras dimensões, implementação mais geral
            // (simplificada - requer sistema linear)
            static_assert(N == 2, "Intersection only implemented for 2D segments");
            return std::nullopt;
        }
    }

    bool intersects(const Segment& other, T eps = static_cast<T>(1e-8)) const {
        return intersection(other, eps).has_value();
    }

    /* ================= RELATIONSHIPS ================= */

    bool contains_point(const PointType& point, T eps = static_cast<T>(1e-8)) const {
        VectorType dir = direction();
        VectorType to_point = point - p0;
        
        // Verifica colinearidade
        if constexpr (N == 2) {
            T cross = dir[0] * to_point[1] - dir[1] * to_point[0];
            if (std::abs(cross) > eps)
                return false;
        } else if constexpr (N == 3) {
            VectorType cross = dir.cross(to_point);
            if (std::abs(cross[0]) > eps || std::abs(cross[1]) > eps || std::abs(cross[2]) > eps)
                return false;
        }
        
        // Verifica se está dentro do segmento
        T dot_dir = dir.dot(to_point);
        T dot_dir_self = dir.dot(dir);
        
        if (dot_dir < -eps || dot_dir > dot_dir_self + eps)
            return false;
        
        return true;
    }

    /* ================= COMPARISONS ================= */

    bool operator==(const Segment& other) const {
        return (p0 == other.p0 && p1 == other.p1) ||
               (p0 == other.p1 && p1 == other.p0);
    }

    bool operator!=(const Segment& other) const {
        return !(*this == other);
    }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Segment& seg) {
        os << "Segment[" << seg.p0 << " -> " << seg.p1 << "]";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T>
using Segment2 = Segment<T, 2>;

template <Arithmetic T>
using Segment3 = Segment<T, 3>;

using Segment2f = Segment2<float>;
using Segment2d = Segment2<double>;
using Segment3f = Segment3<float>;
using Segment3d = Segment3<double>;

} // namespace geometry