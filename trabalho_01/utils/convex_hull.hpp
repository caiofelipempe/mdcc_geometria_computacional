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

    // Extrair vértices
    hullVertices.clear();
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

int sameSide(const std::vector<geometry::Point3f>& points, int a, int b, int c, int ignore_idx) {
    auto edge1 = points[b] - points[a];
    auto edge2 = points[c] - points[a];
    auto normal = edge1.cross3(edge2);
    
    double ref_sign = 0;
    

    if (!(0 == a || 0 == b || 0 == c || 0 == ignore_idx)) {
        auto to_point = points[0] - points[a];
        auto dist = normal.dot(to_point);

        if (fabs(dist) > 1e-9) {
            ref_sign = (dist > 0) ? 1 : -1;
        }
    }

    for (int i = 1; i < points.size(); i++) {
        if (i == a || i == b || i == c || i == ignore_idx) continue;
        
        auto to_point = points[i] - points[a];
        auto dist = normal.dot(to_point);
        
        if (fabs(dist) > 1e-9) {
            double sign = (dist > 0) ? 1 : -1;
            if (sign != ref_sign) return 0;
        }
    }


    return ref_sign;
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
                auto ss = sameSide(points, i, j, k, -1);
                if (ss > 0) {
                    faces.push_back(geometry::TriangleIndices(k, j, i));
                } else if (ss < 0) {
                    faces.push_back(geometry::TriangleIndices(i, j, k));
                }
            }
        }
    }
    
    return geometry::Mesh3f(points, faces);
}

}