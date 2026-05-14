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
    auto normal = edge1.cross(edge2);

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

void bruteForce(geometry::Mesh3f& mesh) {
    auto& points = mesh.getVertices();
    auto& faces  = mesh.getFaces();

    faces.clear();

    int n = static_cast<int>(points.size());
    if (n < 4) return;

    faces.reserve(n * n);

    for (int i = 0; i < n - 2; ++i) {
        for (int j = i + 1; j < n - 1; ++j) {
            for (int k = j + 1; k < n; ++k) {

                auto e1 = points[j] - points[i];
                auto e2 = points[k] - points[i];
                auto normal = e1.cross(e2);

                if (normal.norm() < 1e-6f)
                    continue;

                int ss = sameSide(points, i, j, k);

                if (ss < 0) {
                    faces.push_back({(size_t)i, (size_t)j, (size_t)k});
                }
                else if (ss > 0) {
                    faces.push_back({(size_t)k, (size_t)j, (size_t)i});
                }
            }
        }
    }
}

class BruteForceStep {
public:
    void start(std::vector<std::tuple<std::string, geometry::Mesh3f>>& meshes_in) {
        meshes = &meshes_in;

        mesh_index = 0;
        i = 0; j = 1; k = 2;

        test_point = 0;
        current_side = 0;

        checking = false;
        valid = true;

        finished = false;
        running = true;

        for (auto& [name, mesh] : *meshes) {
            mesh.getFaces().clear();
        }
    }

    void stop() {
        running = false;
    }

    bool step() {
        if (!running || finished || meshes == nullptr || meshes->empty())
            return false;

        while (true) {
            auto& [name, mesh] = (*meshes)[mesh_index];
            auto& points = mesh.getVertices();

            int n = static_cast<int>(points.size());

            if (n < 4) {
                advanceMesh();
                return true;
            }

            // 🔥 inicia triângulo apenas quando necessário
            if (!checking) {
                checking = true;
                valid = true;
                current_side = 0;
                test_point = 0;
            }

            // 🔥 continua do ponto exato onde parou
            while (test_point < n) {
                int t = test_point++;
                if (t == i || t == j || t == k)
                    continue;

                auto& p0 = points[i];
                auto e1 = points[j] - p0;
                auto e2 = points[k] - p0;
                auto normal = e1.cross(e2);

                if (normal.norm() < 1e-6f) {
                    valid = false;
                    break;
                }

                auto to_p = points[t] - p0;
                double dist = normal.dot(to_p);

                if (std::abs(dist) < 1e-9)
                    continue;

                int side = (dist > 0) ? 1 : -1;

                if (current_side == 0) {
                    current_side = side;
                } else if (current_side != side) {
                    valid = false;
                    break;
                }
            }

            // 🔥 terminou análise do triângulo
            if (valid && current_side != 0) {
                if (current_side < 0) {
                    mesh.addFace(i, j, k);
                } else {
                    mesh.addFace(k, j, i);
                }

                // ✅ preparar próximo triângulo
                checking = false;
                advanceIndices(n);

                return true; // 🔥 PARA AQUI → exatamente 1 face
            }

            // segue para próximo triângulo
            checking = false;
            advanceIndices(n);

            if (finished) return false;
        }
    }

    bool isRunning() const { return running; }

private:
    std::vector<std::tuple<std::string, geometry::Mesh3f>>* meshes = nullptr;

    int mesh_index = 0;
    int i = 0, j = 1, k = 2;

    int test_point = 0;
    int current_side = 0;

    bool valid = true;
    bool checking = false;
    bool finished = false;
    bool running = false;

    void advanceIndices(int n) {
        k++;
        if (k >= n) {
            j++;
            if (j >= n - 1) {
                i++;
                if (i >= n - 2) {
                    advanceMesh();
                    return;
                }
                j = i + 1;
            }
            k = j + 1;
        }
    }

    void advanceMesh() {
        mesh_index++;

        if (mesh_index >= (int)meshes->size()) {
            running = false;
            finished = true;
            return;
        }

        i = 0; j = 1; k = 2;

        checking = false;
        test_point = 0;
        current_side = 0;
        valid = true;
    }
};

}