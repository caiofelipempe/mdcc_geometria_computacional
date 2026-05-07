#pragma once

#include <vector>
#include <memory>
#include <array>
#include "point.hpp"
#include "vector.hpp"
#include "segment.hpp"

namespace geometry {

/**
 * @brief Estrutura para representar uma face triangular por índices de vértices.
 */
struct TriangleIndices {
    std::size_t v0, v1, v2;

    std::size_t operator[](std::size_t i) const {
        return (&v0)[i];
    }
};

/**
 * @brief Classe Mesh genérica para N dimensões.
 * Armazena vértices como Point<T, N> e faces como triângulos de índices.
 */
template <Arithmetic T, std::size_t N>
class Mesh {
public:
    using PointType = Point<T, N>;
    using VectorType = Vector<T, N>;

private:
    std::vector<PointType> vertices;
    std::vector<TriangleIndices> faces;

public:
    Mesh() = default;

    /* ================= MANIPULAÇÃO DE DADOS ================= */

    std::size_t add_vertex(const PointType& p) {
        vertices.push_back(p);
        return vertices.size() - 1;
    }

    void add_face(std::size_t i0, std::size_t i1, std::size_t i2) {
        // Opcional: validação de índices em debug mode
        faces.push_back({i0, i1, i2});
    }

    const std::vector<PointType>& get_vertices() const { return vertices; }
    const std::vector<TriangleIndices>& get_faces() const { return faces; }

    void clear() {
        vertices.clear();
        faces.clear();
    }

    /* ================= PROPRIEDADES GEOMÉTRICAS ================= */

    /**
     * @brief Calcula a área total da mesh. 
     * Funciona para qualquer N >= 2 (área do triângulo no espaço N-dimensional).
     */
    T total_area() const {
        T area = 0;
        for (const auto& face : faces) {
            area += face_area(face);
        }
        return area;
    }

    /**
     * @brief Área de uma face específica usando a fórmula de Heron ou norma do cross product.
     */
    T face_area(const TriangleIndices& f) const {
        VectorType ab = vertices[f.v1] - vertices[f.v0];
        VectorType ac = vertices[f.v2] - vertices[f.v0];
        
        if constexpr (N == 2) {
            // Em 2D, cross product retorna escalar
            return std::abs(ab[0] * ac[1] - ab[1] * ac[0]) * static_cast<T>(0.5);
        } else if constexpr (N == 3) {
            // Em 3D, norma do cross product
            return ab.cross(ac).norm() * static_cast<T>(0.5);
        } else {
            // Caso geral para N dimensões: Area = 0.5 * sqrt(|ab|^2 * |ac|^2 - (ab . ac)^2)
            T dot_ab = ab.dot(ab);
            T dot_ac = ac.dot(ac);
            T dot_ab_ac = ab.dot(ac);
            return std::sqrt(std::max(T{0}, dot_ab * dot_ac - dot_ab_ac * dot_ab_ac)) * static_cast<T>(0.5);
        }
    }

    /**
     * @brief Calcula o baricentro (centro de massa) da mesh.
     */
    PointType centroid() const {
        if (vertices.empty()) return PointType();
        
        PointType center;
        if constexpr (N == 0) center = PointType(vertices[0].size());

        for (const auto& v : vertices) {
            center = center + VectorType(v);
        }
        return center / static_cast<T>(vertices.size());
    }

    /* ================= OPERAÇÕES DE TRANSFORMAÇÃO ================= */

    void translate(const VectorType& translation) {
        for (auto& v : vertices) {
            v = v + translation;
        }
    }

    void scale(T factor) {
        PointType c = centroid();
        for (auto& v : vertices) {
            VectorType dir = v - c;
            v = c + (dir * factor);
        }
    }

    /* ================= BOUNDING BOX ================= */

    std::pair<PointType, PointType> bounding_box() const {
        if (vertices.empty()) return {};

        PointType min_p = vertices[0];
        PointType max_p = vertices[0];

        for (const auto& v : vertices) {
            for (std::size_t i = 0; i < v.size(); ++i) {
                min_p[i] = std::min(min_p[i], v[i]);
                max_p[i] = std::max(max_p[i], v[i]);
            }
        }
        return {min_p, max_p};
    }

    /* ================= OUTPUT ================= */

    friend std::ostream& operator<<(std::ostream& os, const Mesh& mesh) {
        os << "Mesh{V: " << mesh.vertices.size() << ", F: " << mesh.faces.size() << "}";
        return os;
    }
};

/* ================= ALIASES ================= */

template <Arithmetic T> using Mesh2 = Mesh<T, 2>;
template <Arithmetic T> using Mesh3 = Mesh<T, 3>;

using Mesh2f = Mesh2<float>;
using Mesh3f = Mesh3<float>;
using Mesh2d = Mesh2<double>;
using Mesh3d = Mesh3<double>;

} // namespace geometry