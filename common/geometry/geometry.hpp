#pragma once

#include "error.hpp"

#include <array>
#include <vector>
#include <cstddef>
#include <type_traits>
#include <cmath>
#include <expected>
#include <optional>
#include <stdexcept>
#include <algorithm>


namespace geometry {

// ------------------------------------------------------------
// Base aliases & helpers
// ------------------------------------------------------------

enum class Orientation {
    Colinear,
    Clockwise,
    CounterClockwise
};

// Vetor e ponto
template <typename T, std::size_t N>
using Vec = std::array<T, N>;

template <typename T, std::size_t N>
using Point = Vec<T, N>;

// Símplex genérico
template <typename T, std::size_t N>
using Simplex = std::array<Point<T, N>, N + 1>;

// Casos clássicos
template <typename T, std::size_t N>
using Segment = Vec<Point<T, N>, 2>;

template <typename T, std::size_t N>
using Triangle = std::array<Point<T, N>, 3>;

template <typename T>
using Tetraedro = Simplex<T, 3>;

// Pontos específicos
template <typename T>
using Point2 = Vec<T, 2>;

template <typename T>
using Point3 = Vec<T, 3>;

template <typename T>
using Quaternion = Vec<T, 4>;

// VectorOrArray dinâmico (N == 0) ou estático (N > 0)
template <typename T, std::size_t N>
using VectorOrArray = std::conditional_t<
    N == 0,
    std::vector<T>,
    std::array<T, N>
>;

// Polígono (lista de pontos em sequência)
template <typename T, std::size_t N = 0>
using Polygon = VectorOrArray<Point<T, 2>, N>;

// Poliedro por boundary representation
template <typename T, std::size_t VN = 0, std::size_t FN = 0>
struct Polyhedron {
    VectorOrArray<Point3<T>, VN> vertices;
    VectorOrArray<std::array<std::size_t, 3>, FN> faces;
};

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
constexpr T default_epsilon() noexcept {
    if constexpr (std::is_floating_point_v<T>)
        return static_cast<T>(1e-8);
    else
        return T{};
}

// ------------------------------------------------------------
// Arithmetic operators
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator+(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = a[i] + b[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator-(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = a[i] - b[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator-(const Vec<T, N>& v) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = -v[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator*(const Vec<T, N>& a, Vec<T, N>& b) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = a[i] * b[i];
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator*(const Vec<T, N>& v, T s) noexcept {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = v[i] * s;
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator*(T s, const Vec<T, N>& v) noexcept {
    return v * s;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator/(const Vec<T, N>& a, const Vec<T, N>& b) {
    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
    {
        if constexpr (!std::is_floating_point_v<T>)
            if (b[i] == T{}) throw std::domain_error("Division by zero");
        r[i] = a[i] / b[i];
    }
    return r;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N> operator/(const Vec<T, N>& v, T s) {
    if constexpr (!std::is_floating_point_v<T>)
        if (s == T{}) throw std::domain_error("Division by zero");

    Vec<T, N> r{};
    for (std::size_t i = 0; i < N; ++i)
        r[i] = v[i] / s;
    return r;
}

// Compound-assignment operators (necessários para normalize in-place)

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator+=(Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        a[i] += b[i];
    return a;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator-=(Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        a[i] -= b[i];
    return a;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator*=(Vec<T, N>& v, T s) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        v[i] *= s;
    return v;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator*=(Vec<T, N>& a, Vec<T, N> b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        a[i] *= b[i];
    return a;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator/=(Vec<T, N>& v, T s) {
    if constexpr (!std::is_floating_point_v<T>)
        if (s == T{}) throw std::domain_error("Division by zero");
    for (std::size_t i = 0; i < N; ++i)
        v[i] /= s;
    return v;
}

template <Arithmetic T, std::size_t N>
constexpr Vec<T, N>& operator/=(Vec<T, N>& a, Vec<T, N> b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        a[i] /= b[i];
    return a;
}

// ------------------------------------------------------------
// Comparisons
// ------------------------------------------------------------
 
template <Arithmetic T, std::size_t N>
constexpr bool operator==(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    for (std::size_t i = 0; i < N; ++i)
        if (a[i] != b[i]) return false;
    return true;
}

template <Arithmetic T, std::size_t N>
constexpr bool operator!=(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    return !(a == b);
}

// ------------------------------------------------------------
// Dot / length
// ------------------------------------------------------------

template <Arithmetic T, std::size_t N>
constexpr T dot(const Vec<T, N>& a, const Vec<T, N>& b) noexcept {
    T sum{};
    for (std::size_t i = 0; i < N; ++i)
        sum += a[i] * b[i];
    return sum;
}

template <Arithmetic T, std::size_t N>
constexpr T length_squared(const Vec<T, N>& v) noexcept {
    return dot(v, v);
}

template <Arithmetic T, std::size_t N>
T length(const Vec<T, N>& v) {
    if constexpr (std::is_floating_point_v<T>)
        return std::sqrt(length_squared(v));
    else
        return static_cast<T>(
            std::sqrt(static_cast<double>(length_squared(v)))
        );
}

// ------------------------------------------------------------
// Normalization
// ------------------------------------------------------------

// Retorna cópia normalizada, ou erro se o vetor for zero.
template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalized(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));
    return v / std::sqrt(ls);
}

// Normaliza in-place. Retorna erro se o vetor for zero.
template <Arithmetic T, std::size_t N>
std::optional<Error>
normalize(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return Error::make("Vector is zero");
    v /= std::sqrt(ls);
    return std::nullopt;
}

// Normaliza apenas se |v| > 1; vetor zero continua sendo erro.
template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalizedIfGreaterThanOne(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));
    if (ls > T(1))
        return v / std::sqrt(ls);
    return v;
}

template <Arithmetic T, std::size_t N>
std::optional<Error>
normalizeIfGreaterThanOne(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return Error::make("Vector is zero");
    if (ls > T(1))
        v /= std::sqrt(ls);
    return std::nullopt;
}

// Normaliza apenas se |v| < 1 (escala para unitário vetores curtos).
// Vetor zero continua sendo erro.
template <Arithmetic T, std::size_t N>
std::expected<Vec<T, N>, Error>
normalizedIfSmallerThanOne(const Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return std::unexpected(Error::make("Vector is zero"));
    if (ls < T(1))
        return v / std::sqrt(ls);
    return v;
}

template <Arithmetic T, std::size_t N>
std::optional<Error>
normalizeIfSmallerThanOne(Vec<T, N>& v, T eps = default_epsilon<T>()) {
    const T ls = length_squared(v);
    if (ls <= eps * eps)
        return Error::make("Vector is zero");
    if (ls < T(1))
        v /= std::sqrt(ls);
    return std::nullopt;
}

// ------------------------------------------------------------
// Cross product
// ------------------------------------------------------------

template <Arithmetic T>
constexpr T cross(const Vec<T, 2>& a, const Vec<T, 2>& b) noexcept {
    return a[0] * b[1] - a[1] * b[0];
}

template <Arithmetic T>
constexpr Vec<T, 3> cross(const Vec<T, 3>& a, const Vec<T, 3>& b) noexcept {
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

template <Arithmetic T>
constexpr Vec<T, 4> cross(
    const Vec<T, 4>& a,
    const Vec<T, 4>& b,
    const Vec<T, 4>& c
) noexcept {
    return {
        +(a[1] * (b[2] * c[3] - b[3] * c[2])
        - a[2] * (b[1] * c[3] - b[3] * c[1])
        + a[3] * (b[1] * c[2] - b[2] * c[1])),

        -(a[0] * (b[2] * c[3] - b[3] * c[2])
        - a[2] * (b[0] * c[3] - b[3] * c[0])
        + a[3] * (b[0] * c[2] - b[2] * c[0])),

        +(a[0] * (b[1] * c[3] - b[3] * c[1])
        - a[1] * (b[0] * c[3] - b[3] * c[0])
        + a[3] * (b[0] * c[1] - b[1] * c[0])),

        -(a[0] * (b[1] * c[2] - b[2] * c[1])
        - a[1] * (b[0] * c[2] - b[2] * c[0])
        + a[2] * (b[0] * c[1] - b[1] * c[0]))
    };
}

template <Arithmetic T>
std::expected<T, Error>
pseudoangleOctante(const Vec<T, 2>& v, T eps = default_epsilon<T>()) {
    const T x = v[0];
    const T y = v[1];

    if (std::abs(x) <= eps && std::abs(y) <= eps)
        return std::unexpected(Error::make("Vector is zero"));

    const T ax = std::abs(x);
    const T ay = std::abs(y);

    // ax >= ay: octantes 1, 8 (x > 0) ou 4, 5 (x < 0)
    if (ax >= ay) {
        const T t = ay / ax;           // t ∈ [0, 1]
        if (x >= T{})
            return (y >= T{}) ? t              // oct 1: [0, 1)
                              : T(8) - t;      // oct 8: (7, 8]
        else
            return (y >= T{}) ? T(4) - t       // oct 4: (3, 4]
                              : T(4) + t;      // oct 5: [4, 5)
    }
    // ay > ax: octantes 2, 3 (y > 0) ou 6, 7 (y < 0)
    else {
        const T t = ax / ay;           // t ∈ [0, 1)
        if (y >= T{})
            return (x >= T{}) ? T(2) - t       // oct 2: (1, 2]
                              : T(2) + t;      // oct 3: [2, 3)
        else
            return (x >= T{}) ? T(6) + t       // oct 7: [6, 7)
                              : T(6) - t;      // oct 6: (5, 6]
    }
}

template <Arithmetic T>
std::expected<T, Error>
pseudoangleQuadrant(const Vec<T, 2>& v, T eps = default_epsilon<T>()) {
    const T x = v[0];
    const T y = v[1];

    if (std::abs(x) <= eps && std::abs(y) <= eps)
        return std::unexpected(Error::make("Vector is zero"));

    const T ax = std::abs(x);
    const T ay = std::abs(y);
    const T s  = ax + ay;        // sempre > 0

    if (x >= T{}) {
        if (y >= T{}) return y / s;              // Q1: [0, 1)
        else          return T(3) + x / s;       // Q4: [3, 4)
    } else {
        if (y >= T{}) return T(1) + ax / s;      // Q2: [1, 2)
        else          return T(2) + ay / s;      // Q3: [2, 3)
    }
}

// ------------------------------------------------------------
// Utilitários de comparação / diferença angular
// ------------------------------------------------------------

// Comparador CCW para usar em std::sort.
// Vetores zero são enviados para o final (greater-than-all).
template <Arithmetic T>
bool pseudoangle_less(const Vec<T, 2>& a, const Vec<T, 2>& b) {
    const auto pa = pseudoangleOctante(a);
    const auto pb = pseudoangleOctante(b);

    if (!pa.has_value() && !pb.has_value()) return false; // ambos zero: iguais
    if (!pa.has_value()) return false; // zero vai para o fim
    if (!pb.has_value()) return true;  // b é zero, a vem antes

    return pa.value() < pb.value();
}

// Retorna o ângulo "signed" de a até b no pseudoespaço: pb - pa.
// Não normaliza para [0, 8) — o chamador decide a semântica.
template <Arithmetic T>
std::expected<T, Error>
pseudoangleBetween(const Vec<T, 2>& a, const Vec<T, 2>& b) {
    const auto pa = pseudoangleOctante(a);
    const auto pb = pseudoangleOctante(b);

    if (!pa.has_value() || !pb.has_value())
        return std::unexpected(Error::make("One or both vectors are zero"));

    return pb.value() - pa.value();
}

template <typename T>
T orientedArea2(const Triangle<T, 2>& tri) {
    const auto& a = tri[0];
    const auto& b = tri[1];
    const auto& c = tri[2];

    Point<T, 2> ab{ b[0] - a[0], b[1] - a[1] };
    Point<T, 2> ac{ c[0] - a[0], c[1] - a[1] };
    return cross(ab, ac);
}

template <typename T>
Vec<T, 3> triangleNormal(const Triangle<T, 3>& tri) {
    const auto& a = tri[0];
    const auto& b = tri[1];
    const auto& c = tri[2];

    Point<T, 3> ab{ b[0] - a[0], b[1] - a[1], b[2] - a[2] };
    Point<T, 3> ac{ c[0] - a[0], c[1] - a[1], c[2] - a[2] };
    return cross(ab, ac);
}

template <typename T>
bool isZero(T v, T eps) {
    return std::abs(v) <= eps;
}

template <typename T>
bool onSegment(const Segment<T, 2>& s, const Point2<T>& p, T eps = default_epsilon<T>()) {
    const auto& a = s[0];
    const auto& b = s[1];

    return
        p[0] >= std::min(a[0], b[0]) - eps &&
        p[0] <= std::max(a[0], b[0]) + eps &&
        p[1] >= std::min(a[1], b[1]) - eps &&
        p[1] <= std::max(a[1], b[1]) + eps;
}

template <typename T>
bool segmentIntersectionExists(const Segment<T, 2>& s1, const Segment<T, 2>& s2, T eps = default_epsilon<T>()) {
    T o1 = orientedArea2(Triangle{s1[0], s1[1], s2[0]});
    T o2 = orientedArea2(Triangle{s1[0], s1[1], s2[1]});
    T o3 = orientedArea2(Triangle{s2[0], s2[1], s1[0]});
    T o4 = orientedArea2(Triangle{s2[0], s2[1], s1[1]});

    // Interseção própria
    if ((o1 > eps && o2 < -eps || o1 < -eps && o2 > eps) &&
        (o3 > eps && o4 < -eps || o3 < -eps && o4 > eps))
        return true;

    // Casos colineares
    if (isZero(o1, eps) && onSegment(s1, s2[0], eps)) return true;
    if (isZero(o2, eps) && onSegment(s1, s2[1], eps)) return true;
    if (isZero(o3, eps) && onSegment(s2, s1[0], eps)) return true;
    if (isZero(o4, eps) && onSegment(s2, s1[1], eps)) return true;

    return false;
}

template <typename T>
std::optional<Point2<T>> segmentIntersectionPoint(const Segment<T, 2>& s1, const Segment<T, 2>& s2, T eps = default_epsilon<T>()) {
    const auto& p = s1[0];
    const auto& p2 = s1[1];
    const auto& q = s2[0];
    const auto& q2 = s2[1];

    Point2<T> r{ p2[0] - p[0], p2[1] - p[1] };
    Point2<T> s{ q2[0] - q[0], q2[1] - q[1] };

    T rxs = cross(r, s);
    T q_pxs = cross(
        Point2<T>{ q[0] - p[0], q[1] - p[1] },
        s
    );

    if (std::abs(rxs) <= eps)
        return std::nullopt;

    T t = q_pxs / rxs;

    if (t < -eps || t > T(1) + eps)
        return std::nullopt;

    return Point2<T>{
        p[0] + t * r[0],
        p[1] + t * r[1]
    };
}

template <typename T>
std::size_t
segmentPolygonIntersectionCount(
    const Segment<T, 2>& segment,
    const Polygon<T>& polygon,
    T eps = default_epsilon<T>()
) {
    std::size_t count = 0;

    for (std::size_t i = 0; i < polygon.size(); ++i) {
        Segment<T, 2> edge{
            polygon[i],
            polygon[(i + 1) % polygon.size()]
        };

        if (segmentIntersectionExists(segment, edge, eps))
            ++count;
    }

    return count;
}

template <typename T>
bool
segmentPolygonIntersectionExists(
    const Segment<T, 2>& segment,
    const Polygon<T>& polygon,
    T eps = default_epsilon<T>()
) {
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        Segment<T, 2> edge{
            polygon[i],
            polygon[(i + 1) % polygon.size()]
        };

        if (segmentIntersectionExists(segment, edge, eps))
            return true;
    }

    return false;
}

template <typename T>
std::optional<Point2<T>>
segmentPolygonIntersectionFirstPoint(
    const Segment<T, 2>& segment,
    const Polygon<T>& polygon,
    T eps = default_epsilon<T>()
) {
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        Segment<T, 2> edge{
            polygon[i],
            polygon[(i + 1) % polygon.size()]
        };

        if (!segmentIntersectionExists(segment, edge, eps))
            continue;

        auto p = segmentIntersectionPoint(segment, edge, eps);
        if (p)
            return p;
    }

    return std::nullopt;
}

template <typename T, std::size_t N>
T distancePointSegmentSquared(const Vec<T, N>& P, const Vec<T, N>& A, const Vec<T, N>& B)
{
    Vec<T, N> AB = sub(B, A);
    Vec<T, N> AP = sub(P, A);

    T ab2 = dot(AB, AB);

    // Segmento degenerado (A == B)
    if (ab2 == 0.0)
        return dot(AP, AP);

    T t = dot(AP, AB) / ab2;
    t = std::clamp(t, 0.0, 1.0);

    Vec<T, N> d(AP.size());
    for (std::size_t i = 0; i < d.size(); ++i)
        d[i] = AP[i] - t * AB[i];

    return dot(d, d);
}


template <typename T, std::size_t N>
T distancePointSegment(const Vec<T, N>& P, const Vec<T, N>& A, const Vec<T, N>& B) {
    return std::sqrt(distancePointSegmentSquared(P, A, B));
}

template <typename T>
std::vector<Point2<T>>
segmentPolygonIntersectionPoints(
    const Segment<T, 2>& segment,
    const Polygon<T>& polygon,
    T eps = default_epsilon<T>()
) {
    std::vector<Point2<T>> points;

    for (std::size_t i = 0; i < polygon.size(); ++i) {
        Segment<T, 2> edge{
            polygon[i],
            polygon[(i + 1) % polygon.size()]
        };

        if (!segmentIntersectionExists(segment, edge, eps))
            continue;

        auto p = segmentIntersectionPoint(segment, edge, eps);
        if (p)
            points.push_back(*p);
    }

    return points;
}

template <typename T>
bool isPointInsidePolygonRaycast(const Point2<T>& point, const Polygon<T>& polygon, T eps = default_epsilon<T>()) {
    int n = polygon.size();
    bool isInside = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i][1] > point[1]) != (polygon[j][1] > point[1])) &&
            (point[0] < (polygon[j][0] - polygon[i][0]) * (point[1] - polygon[i][1]) / 
            (polygon[j][1] - polygon[i][1]) + polygon[i][0])) {
            isInside = !isInside;
        }
    }

    return isInside;
}

template <typename T>
bool isPointInsidePolygonWinding(const Point2<T>& p, const Polygon<T>& polygon)
{
    int windingNumber = 0;
    size_t n = polygon.size();

    for (size_t i = 0; i < n; ++i) {
        const auto& v1 = polygon[i];
        const auto& v2 = polygon[(i + 1) % n];
        
        // Aresta cruza para cima
        if (v1[1] <= p[1]) {
            if (v2[1] > p[1]) { // cruzamento ascendente
                if (orientedArea2(Triangle{v1, v2, p}) > 0)
                    ++windingNumber;
            }
        }
        // Aresta cruza para baixo
        else {
            if (v2[1] <= p[1]) { // cruzamento descendente
                if (orientedArea2(Triangle{v1, v2, p}) < 0)
                    --windingNumber;
            }
        }
    }

    return windingNumber != 0;
}


} // namespace geometry