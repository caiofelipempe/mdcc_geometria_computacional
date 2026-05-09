#pragma once

#include "mesh.hpp"

#include <vector>
#include <cstdint>
#include <CGAL/Simple_cartesian.h>
#include <map>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/convex_hull_3.h>

typedef CGAL::Simple_cartesian<double> K;

typedef CGAL::Simple_cartesian<double> K;
typedef CGAL::Polyhedron_3<K> CgalPolyhedron;
typedef CgalPolyhedron::Vertex_handle Vertex_handle;
typedef CgalPolyhedron::Facet_handle Facet_handle;
typedef K::Point_3 CgalPoint3;

namespace convex_hull {

geometry::Mesh3f cgal(std::vector<geometry::Point3f> points) {
    std::vector<float> hullVertices;
    std::vector<geometry::TriangleIndices> faces;

    std::vector<CgalPoint3> cgal_points;
    for (int i = 0; i < points.size(); ++i) {
        cgal_points.push_back(CgalPoint3(points[i][0], points[i][1], points[i][2]));
    }
    points.clear();
    
    CgalPolyhedron poly;
    CGAL::convex_hull_3(cgal_points.begin(), cgal_points.end(), poly);

    std::map<Vertex_handle, unsigned int> vertexIndex;
    unsigned int idx = 0;
    for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
        points.push_back(geometry::Point3f({(float)v->point().x(), (float)v->point().y(), (float)v->point().z()}));
        vertexIndex[v] = idx++;
    }


    for (auto f = poly.facets_begin(); f != poly.facets_end(); ++f) {
        std::vector<unsigned int> face;
        auto h = f->facet_begin();
        do {
            face.push_back(vertexIndex[h->vertex()]);
            ++h;
        } while (h != f->facet_begin());
        faces.push_back(geometry::TriangleIndices({face[0], face[1], face[2]}));
    }

    return geometry::Mesh3f(points, faces);
}

int sameSide(const std::vector<geometry::Point3f>& points, int a, int b, int c) {
    auto po = points[a];
    auto edge1 = points[b] - po;
    auto edge2 = points[c] - po;
    auto normal = edge1.cross3(edge2);

    int side = 0;
    for (int i = 0; i < (int)points.size(); i++) {
        if (i == a || i == b || i == c) continue;

        auto to_point = points[i] - po;
        double dist = normal.dot(to_point);

        if (std::abs(dist) > 1e-9) {
            int current_side = (dist > 0) ? 1 : -1;

            if (side == 0) {
                side = current_side;
            } else if (side != current_side) {
                return 0;
            }
        }
    }

    return side;
}

geometry::Mesh3f bruteForce(std::vector<geometry::Point3f> points) {
    int n = points.size();
    if (n < 4) {
        return geometry::Mesh3f();
    }
    
    std::vector<geometry::TriangleIndices> faces;
    
    for (int i = 0; i < n-2; i++) {
        for (int j = i+1; j < n-1; j++) {
            for (int k = j+1; k < n; k++) {
                auto ss = sameSide(points, i, j, k);
                if (ss < 0) {
                    faces.push_back(geometry::TriangleIndices(i, j, k));
                } else if (ss > 0) {
                    faces.push_back(geometry::TriangleIndices(k, j, i));
                }
            }
        }
    }
    
    return geometry::Mesh3f(points, faces);
}

}