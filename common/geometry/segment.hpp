#pragma once

#include <cmath>
#include <ostream>
#include <optional>
#include <algorithm>
#include <stdexcept>

#include "arithmetic.hpp"
#include "point.hpp"
#include "vector.hpp"

namespace geometry {

/**
 * @brief Segmento geométrico definido por dois pontos.
 */
template <Scalar T, std::size_t N>
class Segment {
public:

    using ValueType  = T;
    using PointType  = Point<T, N>;
    using VectorType = Vector<T, N>;

private:

    PointType p0;
    PointType p1;

public:

    /* ================= CONSTRUTORES ================= */

    constexpr Segment() = default;

    constexpr Segment(const PointType& start,
                      const PointType& end)
        : p0(start),
          p1(end)
    {
        if constexpr (N == 0) {

            if (start.size() != end.size()) {

                throw std::runtime_error(
                    "Segment points dimension mismatch"
                );
            }
        }
    }

    Segment(std::initializer_list<PointType> points) {

        if (points.size() != 2) {

            throw std::invalid_argument(
                "Segment requires exactly 2 points"
            );
        }

        auto it = points.begin();

        p0 = *it;
        p1 = *(++it);
    }

    /* ================= ACESSO ================= */

    [[nodiscard]]
    constexpr const PointType&
    start() const noexcept {
        return p0;
    }

    [[nodiscard]]
    constexpr const PointType&
    end() const noexcept {
        return p1;
    }

    constexpr void set_start(
        const PointType& point
    ) {
        p0 = point;
    }

    constexpr void set_end(
        const PointType& point
    ) {
        p1 = point;
    }

    /* ================= PROPRIEDADES ================= */

    [[nodiscard]]
    VectorType direction() const {

        return p1 - p0;
    }

    [[nodiscard]]
    T squared_length() const {

        return p0.squared_distance_to(p1);
    }

    [[nodiscard]]
    T length() const
    requires NormalizableScalar<T>
    {
        return p0.distance_to(p1);
    }

    [[nodiscard]]
    PointType midpoint() const {

        return p0.midpoint(p1);
    }

    [[nodiscard]]
    bool degenerate(T eps = T{}) const {

        return squared_length()
            <=
            (eps * eps);
    }

    /* ================= PARAMETRIZAÇÃO ================= */

    /**
     * @brief t=0 -> p0
     *        t=1 -> p1
     */
    [[nodiscard]]
    PointType at(const T& t) const {

        return p0 + (direction() * t);
    }

    /* ================= DISTÂNCIA ================= */

    [[nodiscard]]
    PointType closest_point_to(
        const PointType& p
    ) const {

        const VectorType dir =
            direction();

        const T l2 =
            dir.dot(dir);

        if (l2 <= T{}) {
            return p0;
        }

        T t =
            (p - p0).dot(dir)
            /
            l2;

        t = std::clamp(
            t,
            T{},
            T{1}
        );

        return at(t);
    }

    [[nodiscard]]
    T squared_distance_to_point(
        const PointType& p
    ) const {

        return p.squared_distance_to(
            closest_point_to(p)
        );
    }

    [[nodiscard]]
    T distance_to_point(
        const PointType& p
    ) const
    requires NormalizableScalar<T>
    {
        using std::sqrt;

        return sqrt(
            squared_distance_to_point(p)
        );
    }

    /* ================= INTERSECÇÃO 2D ================= */

    template<std::size_t M = N>
    requires (M == 2)
    [[nodiscard]]
    std::optional<PointType>
    intersection(
        const Segment& other,
        T eps = static_cast<T>(1e-8)
    ) const {

        const PointType& a = p0;
        const PointType& b = p1;

        const PointType& c = other.p0;
        const PointType& d = other.p1;

        const VectorType r =
            b - a;

        const VectorType s =
            d - c;

        const T rxs =
            r.cross(s);

        using std::abs;

        if (abs(rxs) < eps) {
            return std::nullopt;
        }

        const VectorType qp =
            c - a;

        const T t =
            qp.cross(s)
            /
            rxs;

        const T u =
            qp.cross(r)
            /
            rxs;

        if (
            t >= -eps &&
            t <= T{1} + eps &&
            u >= -eps &&
            u <= T{1} + eps
        ) {

            return at(
                std::clamp(
                    t,
                    T{},
                    T{1}
                )
            );
        }

        return std::nullopt;
    }

    /* ================= RELAÇÕES ================= */

    [[nodiscard]]
    bool contains_point(
        const PointType& p,
        T eps = static_cast<T>(1e-8)
    ) const {

        return
            squared_distance_to_point(p)
            <=
            (eps * eps);
    }

    /* ================= COMPARAÇÃO ================= */

    [[nodiscard]]
    bool operator==(const Segment& other) const {

        return
            (
                p0 == other.p0 &&
                p1 == other.p1
            )
            ||
            (
                p0 == other.p1 &&
                p1 == other.p0
            );
    }

    [[nodiscard]]
    bool operator!=(const Segment& other) const {

        return !(*this == other);
    }

    /* ================= I/O ================= */

    friend std::ostream& operator<<(
        std::ostream& os,
        const Segment& seg
    ) {

        os
            << "Seg["
            << seg.p0
            << " -> "
            << seg.p1
            << "]";

        return os;
    }
};

/* ================= ALIASES ================= */

template <Scalar T>
using Segment2 = Segment<T, 2>;

template <Scalar T>
using Segment3 = Segment<T, 3>;

using Segment2f = Segment2<float>;
using Segment2d = Segment2<double>;

using Segment3f = Segment3<float>;
using Segment3d = Segment3<double>;

} // namespace geometry