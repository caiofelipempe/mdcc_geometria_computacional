#pragma once

using namespace geometry;
#include "point.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

// CGAL (somente se for usar depois)
#include <CGAL/Simple_cartesian.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_2 CgalPoint2;

namespace convex_hull {

// ================= AUX =================

float cross(const Point2f& O, const Point2f& A, const Point2f& B) {
    return (A - O).cross(B - O);
}

float dist2(const Point2f& a, const Point2f& b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    return dx * dx + dy * dy;
}

// distância (área * 2) de P à linha AB
float distance_line(const Point2f& A, const Point2f& B, const Point2f& P) {
    return std::abs(cross(A, B, P));
}

// ================= GRAHAM =================

std::vector<Point2f> graham(std::vector<Point2f> points) {
    int n = points.size();
    if (n < 3) return points;

    int p0 = 0;
    for (int i = 1; i < n; i++) {
        if (points[i][1] < points[p0][1] ||
           (points[i][1] == points[p0][1] && points[i][0] < points[p0][0])) {
            p0 = i;
        }
    }

    std::swap(points[0], points[p0]);
    Point2f pivot = points[0];

    std::sort(points.begin() + 1, points.end(),
        [&](const Point2f& a, const Point2f& b) {
            float c = cross(pivot, a, b);
            if (c == 0)
                return dist2(pivot, a) < dist2(pivot, b);
            return c > 0;
        }
    );

    std::vector<Point2f> hull;
    hull.push_back(points[0]);
    hull.push_back(points[1]);
    hull.push_back(points[2]);

    for (int i = 3; i < n; i++) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }

    return hull;
}

// ================= JARVIS (GIFT WRAPPING) =================

std::vector<Point2f> jarvis(const std::vector<Point2f>& points) {
    int n = points.size();
    if (n < 3) return points;

    std::vector<Point2f> hull;

    int left = 0;
    for (int i = 1; i < n; i++) {
        if (points[i][0] < points[left][0])
            left = i;
    }

    int p = left;
    do {
        hull.push_back(points[p]);
        int q = (p + 1) % n;

        for (int i = 0; i < n; i++) {
            float c = cross(points[p], points[q], points[i]);
            if (c < 0 || (c == 0 && dist2(points[p], points[i]) > dist2(points[p], points[q]))) {
                q = i;
            }
        }

        p = q;

    } while (p != left);

    return hull;
}

// ================= QUICKHULL =================

int find_side(const Point2f& A, const Point2f& B, const Point2f& P) {
    float val = cross(A, B, P);
    if (val > 0) return 1;
    if (val < 0) return -1;
    return 0;
}

float line_dist(const Point2f& A, const Point2f& B, const Point2f& P) {
    return std::abs(cross(A, B, P));
}

void quickhull_rec(const std::vector<Point2f>& pts,
                   const Point2f& A,
                   const Point2f& B,
                   int side,
                   std::vector<Point2f>& hull) {

    int idx = -1;
    float max_dist = 0;

    for (int i = 0; i < pts.size(); i++) {
        float d = line_dist(A, B, pts[i]);

        if (find_side(A, B, pts[i]) == side && d > max_dist) {
            idx = i;
            max_dist = d;
        }
    }

    if (idx == -1) {
        hull.push_back(A);
        return;
    }

    quickhull_rec(pts, pts[idx], A,
                  -find_side(pts[idx], A, B), hull);

    quickhull_rec(pts, pts[idx], B,
                  -find_side(pts[idx], B, A), hull);
}

std::vector<Point2f> quickhull(const std::vector<Point2f>& points) {
    int n = points.size();
    if (n < 3) return points;

    int min_x = 0, max_x = 0;
    for (int i = 1; i < n; i++) {
        if (points[i][0] < points[min_x][0]) min_x = i;
        if (points[i][0] > points[max_x][0]) max_x = i;
    }

    std::vector<Point2f> hull;

    quickhull_rec(points, points[min_x], points[max_x], 1, hull);
    quickhull_rec(points, points[min_x], points[max_x], -1, hull);

    // adicionar último ponto extremo
    hull.push_back(points[max_x]);

    // remover duplicados
    std::sort(hull.begin(), hull.end(),
        [](const Point2f& a, const Point2f& b) {
            if (a[0] != b[0]) return a[0] < b[0];
            return a[1] < b[1];
        });

    hull.erase(std::unique(hull.begin(), hull.end(),
        [](const Point2f& a, const Point2f& b) {
            return a[0] == b[0] && a[1] == b[1];
        }), hull.end());

    return hull;
}

// ================= MERGEHULL =================

// orientação
int orientation(const Point2f& a, const Point2f& b, const Point2f& c) {
    float val = cross(a, b, c);
    if (val > 0) return 1;   // CCW
    if (val < 0) return -1;  // CW
    return 0;
}

// merge de dois hulls convexos (ambos CCW)
std::vector<Point2f> merge_hulls(
    const std::vector<Point2f>& left,
    const std::vector<Point2f>& right) {

    int n1 = left.size();
    int n2 = right.size();

    // ponto mais à direita do left
    int i = 0;
    for (int k = 1; k < n1; k++)
        if (left[k][0] > left[i][0]) i = k;

    // ponto mais à esquerda do right
    int j = 0;
    for (int k = 1; k < n2; k++)
        if (right[k][0] < right[j][0]) j = k;

    // ================= TAMGENTE SUPERIOR =================
    int done = 0;
    int i_up = i, j_up = j;

    while (!done) {
        done = 1;
        while (orientation(right[j_up], left[i_up], left[(i_up+1)%n1]) > 0)
            i_up = (i_up + 1) % n1;

        while (orientation(left[i_up], right[j_up], right[(n2 + j_up - 1)%n2]) < 0) {
            j_up = (n2 + j_up - 1) % n2;
            done = 0;
        }
    }

    // ================= TAMGENTE INFERIOR =================
    done = 0;
    int i_low = i, j_low = j;

    while (!done) {
        done = 1;
        while (orientation(left[i_low], right[j_low], right[(j_low+1)%n2]) > 0)
            j_low = (j_low + 1) % n2;

        while (orientation(right[j_low], left[i_low], left[(n1 + i_low - 1)%n1]) < 0) {
            i_low = (n1 + i_low - 1) % n1;
            done = 0;
        }
    }

    // ================= CONSTRUIR HULL FINAL =================
    std::vector<Point2f> hull;

    // parte esquerda
    int k = i_up;
    hull.push_back(left[k]);
    while (k != i_low) {
        k = (k + 1) % n1;
        hull.push_back(left[k]);
    }

    // parte direita
    k = j_low;
    hull.push_back(right[k]);
    while (k != j_up) {
        k = (k + 1) % n2;
        hull.push_back(right[k]);
    }

    return hull;
}

// função recursiva
std::vector<Point2f> mergehull_rec(std::vector<Point2f>& pts, int l, int r) {
    if (r - l <= 3) {
        std::vector<Point2f> tmp;
        for (int i = l; i <= r; i++) tmp.push_back(pts[i]);
        return graham(tmp); // base simples
    }

    int mid = (l + r) / 2;

    auto left = mergehull_rec(pts, l, mid);
    auto right = mergehull_rec(pts, mid + 1, r);

    return merge_hulls(left, right);
}

std::vector<Point2f> mergehull(std::vector<Point2f> pts) {
    if (pts.size() < 3) return pts;

    std::sort(pts.begin(), pts.end(),
        [](const Point2f& a, const Point2f& b) {
            if (a[0] != b[0]) return a[0] < b[0];
            return a[1] < b[1];
        });

    return mergehull_rec(pts, 0, pts.size() - 1);
}

} // namespace convex_hull
