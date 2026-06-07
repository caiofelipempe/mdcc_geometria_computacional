#pragma once

#include "mesh.hpp"

#include <vector>
#include <cstdint>
#include <iostream>

// CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_data_structure_3.h>

// Kernel
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

// ✅ Vertex com índice
typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_cell_base_3<K> Cb;
typedef CGAL::Triangulation_data_structure_3<Vb, Cb> Tds;

typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef K::Point_3 CgalPoint3;

namespace tetrahedralization {

void cgal(geometry::Mesh3f& mesh) {
    std::vector<std::pair<CgalPoint3, int>> cgal_points;

    for (int i = 0; i < (int)mesh.getVertices().size(); ++i) {
        cgal_points.push_back({
            CgalPoint3(mesh.getVertices()[i][0], mesh.getVertices()[i][1], mesh.getVertices()[i][2]),
            i
        });
    }

    // triangulação
    Delaunay dt;
    dt.insert(cgal_points.begin(), cgal_points.end());

    // ✅ coletando tetraedros
    for (auto cell = dt.finite_cells_begin(); cell != dt.finite_cells_end(); ++cell) {

        int i0 = cell->vertex(0)->info();
        int i1 = cell->vertex(1)->info();
        int i2 = cell->vertex(2)->info();
        int i3 = cell->vertex(3)->info();

        mesh.addTetrahedron(i0, i1, i2, i3);
    }
}

}