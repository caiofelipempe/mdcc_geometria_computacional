#pragma once

#include "mesh.hpp"
#include <array>
#include <set>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_data_structure_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_cell_base_3<K> Cb;
typedef CGAL::Triangulation_data_structure_3<Vb, Cb> Tds;

typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef K::Point_3 CgalPoint3;

namespace tetrahedralization {

using namespace geometry;

void cgal(geometry::Mesh3f& mesh) {
    std::vector<std::pair<CgalPoint3, int>> cgal_points;

    for (int i = 0; i < (int)mesh.getVertices().size(); ++i) {
        cgal_points.push_back({
            CgalPoint3(mesh.getVertices()[i][0], mesh.getVertices()[i][1], mesh.getVertices()[i][2]),
            i
        });
    }

    Delaunay dt;
    dt.insert(cgal_points.begin(), cgal_points.end());

    for (auto cell = dt.finite_cells_begin(); cell != dt.finite_cells_end(); ++cell) {

        int i0 = cell->vertex(0)->info();
        int i1 = cell->vertex(1)->info();
        int i2 = cell->vertex(2)->info();
        int i3 = cell->vertex(3)->info();

        mesh.addTetrahedron(i0, i1, i2, i3);
    }
}

bool compFaces(const std::array<std::size_t, 3>& f1, const std::array<std::size_t, 3>& f2) {
    std::array<std::size_t, 3> t1 = {f1[0], f1[1], f1[2]};
    std::array<std::size_t, 3> t2 = {f2[0], f2[1], f2[2]};
    std::sort(t1.begin(), t1.end());
    std::sort(t2.begin(), t2.end());
    return t1 < t2;
}

struct Circumsphere {
    Point3f center;
    float radius_sqr;
};

float det4x4(float m[4][4]) {
    return m[0][0] * (m[1][1] * (m[2][2]*m[3][3] - m[2][3]*m[3][2]) - m[1][2] * (m[2][1]*m[3][3] - m[2][3]*m[3][1]) + m[1][3] * (m[2][1]*m[3][2] - m[2][2]*m[3][1]))
         - m[0][1] * (m[1][0] * (m[2][2]*m[3][3] - m[2][3]*m[3][2]) - m[1][2] * (m[2][0]*m[3][3] - m[2][3]*m[3][0]) + m[1][3] * (m[2][0]*m[3][2] - m[2][2]*m[3][0]))
         + m[0][2] * (m[1][0] * (m[2][1]*m[3][3] - m[2][3]*m[3][1]) - m[1][1] * (m[2][0]*m[3][3] - m[2][3]*m[3][0]) + m[1][3] * (m[2][0]*m[3][1] - m[2][1]*m[3][0]))
         - m[0][3] * (m[1][0] * (m[2][1]*m[3][2] - m[2][2]*m[3][1]) - m[1][1] * (m[2][0]*m[3][2] - m[2][2]*m[3][0]) + m[1][2] * (m[2][0]*m[3][1] - m[2][1]*m[3][0]));
}

Circumsphere getCircumsphere(const Point3f& p0, const Point3f& p1, const Point3f& p2, const Point3f& p3) {
    float x1 = p1[0] - p0[0], y1 = p1[1] - p0[1], z1 = p1[2] - p0[2];
    float x2 = p2[0] - p0[0], y2 = p2[1] - p0[1], z2 = p2[2] - p0[2];
    float x3 = p3[0] - p0[0], y3 = p3[1] - p0[1], z3 = p3[2] - p0[2];

    float l1 = x1*x1 + y1*y1 + z1*z1;
    float l2 = x2*x2 + y2*y2 + z2*z2;
    float l3 = x3*x3 + y3*y3 + z3*z3;

    float a = x1*(y2*z3 - z2*y3) - y1*(x2*z3 - z2*x3) + z1*(x2*y3 - y2*x3);
    
    if (std::abs(a) < 1e-6f) return { Point3f{0,0,0}, 0.0f }; 

    float Dx = l1*(y2*z3 - z2*y3) - y1*(l2*z3 - z2*l3) + z1*(l2*y3 - y2*l3);
    float Dy = x1*(l2*z3 - z2*l3) - l1*(x2*z3 - z2*x3) + z1*(x2*l3 - l2*x3);
    float Dz = x1*(y2*l3 - l2*y3) - y1*(x2*l3 - l2*x3) + l1*(x2*y3 - y2*x3);

    float cx = Dx / (2.0f * a);
    float cy = Dy / (2.0f * a);
    float cz = Dz / (2.0f * a);

    Circumsphere cs;
    cs.center = Point3f{cx + p0[0], cy + p0[1], cz + p0[2]};
    cs.radius_sqr = cx*cx + cy*cy + cz*cz;
    return cs;
}

void delaunay(geometry::Mesh3f& mesh) {
    if (mesh.vertexCount() < 4) return;

    std::vector<Point3f> local_vertices = mesh.getVertices();
    std::size_t num_original_vertices = local_vertices.size();

    auto [min_p, max_p] = mesh.boundingBox();
    float dx = max_p[0] - min_p[0];
    float dy = max_p[1] - min_p[1];
    float dz = max_p[2] - min_p[2];
    float delta_max = std::max({dx, dy, dz});
    Point3f mid = mesh.centroid();


    std::size_t st0 = local_vertices.size();
    local_vertices.push_back(Point3f{mid[0] - 20 * delta_max, mid[1] - 20 * delta_max, mid[2] - 20 * delta_max});
    
    std::size_t st1 = local_vertices.size();
    local_vertices.push_back(Point3f{mid[0] + 20 * delta_max, mid[1] - 20 * delta_max, mid[2] - 20 * delta_max});
    
    std::size_t st2 = local_vertices.size();
    local_vertices.push_back(Point3f{mid[0], mid[1] + 20 * delta_max, mid[2] - 20 * delta_max});
    
    std::size_t st3 = local_vertices.size();
    local_vertices.push_back(Point3f{mid[0], mid[1], mid[2] + 20 * delta_max});


    std::vector<std::array<std::size_t, 4>> tetrahedrons;
    tetrahedrons.push_back({st0, st1, st2, st3});

    for (std::size_t i = 0; i < num_original_vertices; ++i) {
        const auto& p = local_vertices[i];
        std::vector<std::size_t> bad_tetrahedrons;
        std::set<std::array<std::size_t, 3>> polygon_cavity;

        for (std::size_t t = 0; t < tetrahedrons.size(); ++t) {
            auto cs = getCircumsphere(local_vertices[tetrahedrons[t][0]], 
                                      local_vertices[tetrahedrons[t][1]], 
                                      local_vertices[tetrahedrons[t][2]], 
                                      local_vertices[tetrahedrons[t][3]]);
            
            float dist_sqr = p.squared_distance_to(cs.center);

            if (dist_sqr < cs.radius_sqr - 1e-6f) {
                bad_tetrahedrons.push_back(t);
                
                std::array<std::array<std::size_t, 3>, 4> faces = {
                    {
                        {tetrahedrons[t][0], tetrahedrons[t][1], tetrahedrons[t][2]},
                        {tetrahedrons[t][0], tetrahedrons[t][1], tetrahedrons[t][3]},
                        {tetrahedrons[t][0], tetrahedrons[t][2], tetrahedrons[t][3]},
                        {tetrahedrons[t][1], tetrahedrons[t][2], tetrahedrons[t][3]}
                    }
                };

                for (const auto& face : faces) {
                    if (polygon_cavity.contains(face)) {
                        polygon_cavity.erase(face);
                    } else {
                        polygon_cavity.insert(face);
                    }
                }
            }
        }

        std::sort(bad_tetrahedrons.begin(), bad_tetrahedrons.end(), std::greater<std::size_t>());
        for (auto index : bad_tetrahedrons) {
            tetrahedrons.erase(tetrahedrons.begin() + index);
        }

        for (const auto& face : polygon_cavity) {
            tetrahedrons.push_back({face[0], face[1], face[2], i});
        }
    }

    for (const auto& tet : tetrahedrons) {
        if (tet[0] >= num_original_vertices || 
            tet[1] >= num_original_vertices ||
            tet[2] >= num_original_vertices || 
            tet[3] >= num_original_vertices) {
            continue;
        }
        mesh.addTetrahedron(tet[0], tet[1], tet[2], tet[3]);
    }
}

}