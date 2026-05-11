#pragma once

#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <cmath>

#include "point.hpp"
#include "vector.hpp"
#include "segment.hpp"

namespace geometry {

/* ================= TRIANGLE ================= */

struct TriangleIndices {

    std::size_t v0{};
    std::size_t v1{};
    std::size_t v2{};

    constexpr std::size_t operator[](
        std::size_t i
    ) const {

        return (&v0)[i];
    }
};

/* ================= MESH ================= */

template <Scalar T, std::size_t N>
class Mesh {
public:

    using ValueType  = T;
    using PointType  = Point<T, N>;
    using VectorType = Vector<T, N>;

private:

    std::vector<PointType> vertices;
    std::vector<TriangleIndices> faces;

public:

    /* ================= CONSTRUTORES ================= */

    Mesh() = default;

    Mesh(std::vector<PointType> verts,
         std::vector<TriangleIndices> tris)
        : vertices(std::move(verts)),
          faces(std::move(tris)) {}

    /* ================= DADOS ================= */

    [[nodiscard]]
    std::size_t vertexCount() const noexcept {
        return vertices.size();
    }

    [[nodiscard]]
    std::size_t faceCount() const noexcept {
        return faces.size();
    }

    [[nodiscard]]
    const std::vector<PointType>&
    getVertices() const noexcept {
        return vertices;
    }

    [[nodiscard]]
    const std::vector<TriangleIndices>&
    getFaces() const noexcept {
        return faces;
    }

    [[nodiscard]]
    std::vector<PointType>&
    getVertices() noexcept {
        return vertices;
    }

    [[nodiscard]]
    std::vector<TriangleIndices>&
    getFaces() noexcept {
        return faces;
    }

    void clear() {

        vertices.clear();
        faces.clear();
    }

    /* ================= ADIÇÃO ================= */

    [[nodiscard]]
    std::size_t addVertex(const PointType& p) {

        vertices.push_back(p);

        return vertices.size() - 1;
    }

    void addFace(std::size_t i0,
                 std::size_t i1,
                 std::size_t i2) {

#ifndef NDEBUG

        const auto sz = vertices.size();

        if (i0 >= sz || i1 >= sz || i2 >= sz)
            throw std::out_of_range(
                "Mesh face index out of bounds"
            );

#endif

        faces.push_back({
            i0,
            i1,
            i2
        });
    }

    /* ================= ÁREA ================= */

    [[nodiscard]]
    T faceArea(const TriangleIndices& f) const
    requires NormalizableScalar<T>
    {
        const VectorType ab =
            vertices[f.v1] - vertices[f.v0];

        const VectorType ac =
            vertices[f.v2] - vertices[f.v0];

        if constexpr (N == 2) {

            return
                std::abs(ab.cross(ac))
                *
                T{0.5};

        } else if constexpr (N == 3) {

            return
                ab.cross(ac).norm()
                *
                T{0.5};

        } else {

            const T dot_ab =
                ab.dot(ab);

            const T dot_ac =
                ac.dot(ac);

            const T dot_ab_ac =
                ab.dot(ac);

            const T det =
                dot_ab * dot_ac
                -
                dot_ab_ac * dot_ab_ac;

            using std::sqrt;

            return
                sqrt(std::max(T{}, det))
                *
                T{0.5};
        }
    }

    [[nodiscard]]
    T totalArea() const
    requires NormalizableScalar<T>
    {
        T area{};

        for (const auto& face : faces) {

            area =
                area
                +
                faceArea(face);
        }

        return area;
    }

    /* ================= CENTROIDE ================= */

    [[nodiscard]]
    PointType centroid() const {

        if (vertices.empty()) {

            if constexpr (N == 0)
                return PointType{0};

            return PointType{};
        }

        VectorType accum;

        if constexpr (N == 0) {
            accum = VectorType(vertices[0].size());
        }

        for (const auto& v : vertices) {

            accum += v.to_vector();
        }

        accum /= T(vertices.size());

        return PointType{accum};
    }

    /* ================= TRANSFORMAÇÕES ================= */

    void translate(const VectorType& offset) {

        for (auto& v : vertices) {
            v += offset;
        }
    }

    void scale(const T& factor) {

        const PointType c =
            centroid();

        for (auto& v : vertices) {

            const VectorType dir =
                v - c;

            v =
                c
                +
                (dir * factor);
        }
    }

    template<std::size_t M = N>
    requires (M == 3)
    void rotate(const Vector<T, 4>& q) {

        const PointType c =
            centroid();

        for (auto& v : vertices) {

            VectorType dir =
                v - c;

            dir =
                dir.rotated(q);

            v =
                c + dir;
        }
    }

    /* ================= BOUNDING BOX ================= */

    [[nodiscard]]
    std::pair<PointType, PointType>
    boundingBox() const {

        if (vertices.empty()) {
            return {};
        }

        PointType min_p =
            vertices.front();

        PointType max_p =
            vertices.front();

        for (const auto& v : vertices) {

            for (std::size_t i = 0;
                 i < v.size();
                 ++i) {

                min_p[i] =
                    std::min(
                        min_p[i],
                        v[i]
                    );

                max_p[i] =
                    std::max(
                        max_p[i],
                        v[i]
                    );
            }
        }

        return {
            min_p,
            max_p
        };
    }

    /* ================= RESERVA ================= */

    void reserveVertices(std::size_t n) {
        vertices.reserve(n);
    }

    void reserveFaces(std::size_t n) {
        faces.reserve(n);
    }

    /* ================= VALIDAÇÃO ================= */

    [[nodiscard]]
    bool empty() const noexcept {

        return vertices.empty();
    }

    [[nodiscard]]
    bool valid() const noexcept {

        const auto sz =
            vertices.size();

        for (const auto& f : faces) {

            if (f.v0 >= sz ||
                f.v1 >= sz ||
                f.v2 >= sz) {

                return false;
            }
        }

        return true;
    }

    /* ================= I/O ================= */

    friend std::ostream& operator<<(
        std::ostream& os,
        const Mesh& mesh
    ) {

        os
            << "Mesh{ V: "
            << mesh.vertices.size()
            << ", F: "
            << mesh.faces.size()
            << " }";

        return os;
    }
};

/* ================= ALIASES ================= */

template <Scalar T>
using Mesh2 = Mesh<T, 2>;

template <Scalar T>
using Mesh3 = Mesh<T, 3>;

using Mesh2f = Mesh2<float>;
using Mesh2d = Mesh2<double>;

using Mesh3f = Mesh3<float>;
using Mesh3d = Mesh3<double>;

} // namespace geometry