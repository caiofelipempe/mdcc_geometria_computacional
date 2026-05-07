#pragma once

#include <cmath>
#include <array>
#include <algorithm>
#include <optional>
#include "arithmetic.hpp"
#include "point.hpp"
#include "vector.hpp"

namespace geometry {

/**
 * @brief Classe Triangle definida por três pontos (vértices).
 */
template <Arithmetic T, std::size_t N>
class Triangle {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;

private:
    std::array<PointType, 3> vertices;

public:
    /* ================= CONSTRUTORES ================= */

    Triangle() = default;

    Triangle(const PointType& v0, const PointType& v1, const PointType& v2)
        : vertices{v0, v1, v2} {
        if constexpr (N == 0) {
            if (v0.size() != v1.size() || v1.size() != v2.size())
                throw std::runtime_error("Size mismatch entre os vértices do triângulo");
        }
    }

    /* ================= ACESSO ================= */

    const PointType& operator[](std::size_t i) const { return vertices.at(i); }
    PointType& operator[](std::size_t i) { return vertices.at(i); }

    /* ================= PROPRIEDADES GEOMÉTRICAS ================= */

    /**
     * @brief Calcula a área do triângulo em N dimensões.
     */
    T area() const {
        VectorType ab = vertices[1] - vertices[0];
        VectorType ac = vertices[2] - vertices[0];

        if constexpr (N == 2) {
            // Produto vetorial 2D (determinante)
            return std::abs(ab[0] * ac[1] - ab[1] * ac[0]) * static_cast<T>(0.5);
        } else if constexpr (N == 3) {
            // Norma do produto vetorial 3D
            return ab.cross(ac).norm() * static_cast<T>(0.5);
        } else {
            // Fórmula geral: 0.5 * sqrt(|ab|^2 * |ac|^2 - (ab . ac)^2)
            T dot_ab = ab.dot(ab);
            T dot_ac = ac.dot(ac);
            T dot_ab_ac = ab.dot(ac);
            T val = dot_ab * dot_ac - dot_ab_ac * dot_ab_ac;
            return std::sqrt(std::max(T{0}, val)) * static_cast<T>(0.5);
        }
    }

    PointType centroid() const {
        return (vertices[0] + VectorType(vertices[1]) + VectorType(vertices[2])) / static_cast<T>(3);
    }

    VectorType normal() const {
        static_assert(N >= 3 || N == 0, "Normal só é definida para N >= 3");
        VectorType ab = vertices[1] - vertices[0];
        VectorType ac = vertices[2] - vertices[0];
        
        if constexpr (N == 3) {
            return ab.cross(ac).normalized();
        } else {
            // Para N > 3, o conceito de "a" normal é ambíguo, 
            // mas podemos retornar um vetor ortogonal ao plano do triângulo.
            throw std::runtime_error("Normal única não definida para N > 3");
        }
    }

    /* ================= COORDENADAS BARICÊNTRICAS ================= */

    /**
     * @brief Calcula coordenadas baricêntricas (u, v, w) para um ponto P.
     * P = u*V0 + v*V1 + w*V2, onde u + v + w = 1.
     */
    Vector3<T> barycentric_coords(const PointType& p) const {
        VectorType v0 = vertices[1] - vertices[0];
        VectorType v1 = vertices[2] - vertices[0];
        VectorType v2 = p - vertices[0];

        T d00 = v0.dot(v0);
        T d01 = v0.dot(v1);
        T d11 = v1.dot(v1);
        T d20 = v2.dot(v0);
        T d21 = v2.dot(v1);

        T denom = d00 * d11 - d01 * d01;
        if (std::abs(denom) < static_cast<T>(1e-12)) {
            return Vector3<T>{T{-1}, T{-1}, T{-1}}; // Triângulo degenerado
        }

        T v = (d11 * d20 - d01 * d21) / denom;
        T w = (d00 * d21 - d01 * d20) / denom;
        T u = static_cast<T>(1) - v - w;

        return Vector3<T>{u, v, w};
    }

    /* ================= TESTES DE PONTO ================= */

    bool contains_point(const PointType& p, T eps = static_cast<T>(1e-8)) const {
        Vector3<T> coords = barycentric_coords(p);
        
        // Verifica se o ponto está no plano do triângulo (em N > 2)
        if constexpr (N >= 3) {
            PointType projected = vertices[0] * coords[0] + 
                                  VectorType(vertices[1] * coords[1]) + 
                                  VectorType(vertices[2] * coords[2]);
            if (p.squared_distance_to(projected) > eps) return false;
        }

        return (coords[0] >= -eps && coords[1] >= -eps && coords[2] >= -eps);
    }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Triangle& tri) {
        os << "Triangle{" << tri.vertices[0] << ", " 
           << tri.vertices[1] << ", " << tri.vertices[2] << "}";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T> using Triangle2 = Triangle<T, 2>;
template <Arithmetic T> using Triangle3 = Triangle<T, 3>;

using Triangle2f = Triangle2<float>;
using Triangle3f = Triangle3<float>;
using Triangle2d = Triangle2<double>;
using Triangle3d = Triangle3<double>;

} // namespace geometry