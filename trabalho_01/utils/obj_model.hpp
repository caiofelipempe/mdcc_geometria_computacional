#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "point.hpp"
#include "vector.hpp"
#include "arithmetic.hpp"

struct VertexIndices {
    long long v = -1, vt = -1, vn = -1;
};

struct ObjFace {
    std::vector<VertexIndices> vertices;
};

struct ObjGroup {
    std::string name;
    std::vector<ObjFace> faces;
};

struct ObjObject {
    std::string name;
    std::vector<ObjGroup> groups;

    // Helper para garantir que sempre haja um grupo ativo
    ObjGroup& get_last_group() {
        if (groups.empty()) groups.push_back({"default"});
        return groups.back();
    }
};

template <geometry::Arithmetic T, std::size_t N>
class ObjModel {
public:
    // Dados globais do arquivo
    std::vector<geometry::Point<T, N>> vertices;
    std::vector<geometry::Point<T, 2>> tex_coords; // vt (sempre 2D)
    std::vector<geometry::Vector<T, N>> normals;   // vn

    // Estrutura hierárquica
    std::vector<ObjObject> objects;

    ObjModel() = default;

    /* ================= PARSER ================= */

    static ObjModel load(const std::string& filename) {
        ObjModel model;
        std::ifstream file(filename);
        if (!file.is_open()) return model;

        std::string line;
        // Objeto e grupo padrão iniciais
        model.objects.push_back({"default_object"});
        model.objects.back().groups.push_back({"default_group"});

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                geometry::Point<T, N> p;
                for (std::size_t i = 0; i < (N == 0 ? 3 : N); ++i) ss >> p[i];
                model.vertices.push_back(p);
            }
            else if (prefix == "vt") {
                geometry::Point<T, 2> vt;
                ss >> vt[0] >> vt[1];
                model.tex_coords.push_back(vt);
            }
            else if (prefix == "vn") {
                geometry::Vector<T, N> vn;
                for (std::size_t i = 0; i < (N == 0 ? 3 : N); ++i) ss >> vn[i];
                model.normals.push_back(vn);
            }
            else if (prefix == "o") {
                std::string name;
                ss >> name;
                model.objects.push_back({name});
            }
            else if (prefix == "g") {
                std::string name;
                ss >> name;
                // Adiciona grupo ao objeto atual
                model.objects.back().groups.push_back({name});
            }
            else if (prefix == "f") {
                ObjFace face;
                std::string v_chunk;
                while (ss >> v_chunk) {
                    face.vertices.push_back(parse_vertex_indices(v_chunk));
                }
                model.objects.back().get_last_group().faces.push_back(face);
            }
        }
        return model;
    }

    /* ================= EXPORTADOR ================= */

    void save(const std::string& filename) const {
        std::ofstream file(filename);
        
        file << "# Exported by Geometry Engine\n";

        for (const auto& v : vertices) {
            file << "v";
            for (std::size_t i = 0; i < v.size(); ++i) file << " " << v[i];
            file << "\n";
        }

        for (const auto& vt : tex_coords) file << "vt " << vt[0] << " " << vt[1] << "\n";
        for (const auto& vn : normals) {
            file << "vn";
            for (std::size_t i = 0; i < vn.size(); ++i) file << " " << vn[i];
            file << "\n";
        }

        for (const auto& obj : objects) {
            file << "o " << obj.name << "\n";
            for (const auto& grp : obj.groups) {
                file << "g " << grp.name << "\n";
                for (const auto& face : grp.faces) {
                    file << "f";
                    for (const auto& vi : face.vertices) {
                        file << " " << vi.v;
                        if (vi.vt != -1 || vi.vn != -1) {
                            file << "/";
                            if (vi.vt != -1) file << vi.vt;
                            if (vi.vn != -1) file << "/" << vi.vn;
                        }
                    }
                    file << "\n";
                }
            }
        }
    }

    static ObjModel fromMeshes(const std::vector<std::tuple<std::string, geometry::Mesh3f>>& objects_data)
        requires (N == 3)
    {
        ObjModel model;
        
        if (objects_data.empty()) return model;
        
        std::size_t vertex_offset = 1;
        std::size_t normal_offset = 1;
        std::size_t texcoord_offset = 1;
        
        for (const auto& obj_data : objects_data) {
            const std::string& object_name = std::get<0>(obj_data);
            const geometry::Mesh<T, N>& mesh = std::get<1>(obj_data);
            
            model.objects.push_back({object_name});
            auto& current_obj = model.objects.back();
            
            current_obj.groups.push_back({"mesh_group"});
            auto& current_group = current_obj.groups.back();
            
            const auto& mesh_vertices = mesh.getVertices();
            
            struct VertexNormalKey {
                std::size_t vertex_idx;
                geometry::Vector<T, N> normal;
                bool operator<(const VertexNormalKey& other) const {
                    if (vertex_idx != other.vertex_idx) return vertex_idx < other.vertex_idx;
                    for (std::size_t i = 0; i < N; ++i) {
                        if (normal[i] != other.normal[i]) return normal[i] < other.normal[i];
                    }
                    return false;
                }
            };
            
            std::map<VertexNormalKey, std::size_t> vertex_normal_map;
            
            const auto& faces = mesh.getFaces();
            
            for (const auto& face : faces) {
                const auto& v0 = mesh_vertices[face.v0];
                const auto& v1 = mesh_vertices[face.v1];
                const auto& v2 = mesh_vertices[face.v2];
                
                geometry::Vector<T, N> edge1 = v1 - v0;
                geometry::Vector<T, N> edge2 = v2 - v0;
                geometry::Vector<T, N> face_normal = edge1.cross3(edge2).normalized();
                
                if (face_normal.norm() < T{1e-6}) {
                    face_normal = geometry::Vec3({T{0}, T{0}, T{0}});
                    if constexpr (N == 3) {
                        face_normal[2] = T{1};
                    }
                }
                
                std::vector<VertexIndices> face_vertices;
                
                for (std::size_t i = 0; i < 3; ++i) {
                    std::size_t vertex_idx = face[i];
                    const auto& vertex = mesh_vertices[vertex_idx];
                    
                    VertexNormalKey key{vertex_idx, face_normal};
                    
                    auto it = vertex_normal_map.find(key);
                    if (it != vertex_normal_map.end()) {
                        face_vertices.push_back({static_cast<long long>(it->second), -1, static_cast<long long>(it->second)});
                    } else {
                        model.vertices.push_back(vertex);
                        model.normals.push_back(face_normal);
                        
                        std::size_t new_idx = model.vertices.size();
                        vertex_normal_map[key] = new_idx;
                        
                        face_vertices.push_back({static_cast<long long>(new_idx), -1, static_cast<long long>(new_idx)});
                    }
                }
                
                if (face_vertices.size() == 3) {
                    current_group.faces.push_back({face_vertices});
                }
            }
        }
        
        return model;
    }

    /**
     * @brief Retorna todos os vértices únicos do modelo.
     */
    std::vector<geometry::Point<T, N>> getAllVertices() const {
        return vertices;
    }

    /**
     * @brief Retorna todas as arestas únicas do modelo (sem duplicatas).
     * @return Vetor de pares de pontos (arestas)
     */
    std::vector<std::pair<geometry::Point<T, N>, geometry::Point<T, N>>> getAllEdges() const {
        std::vector<std::pair<geometry::Point<T, N>, geometry::Point<T, N>>> edges;
        
        // Usar um conjunto para evitar arestas duplicadas
        std::set<std::pair<std::size_t, std::size_t>> edgeSet;
        
        for (const auto& object : objects) {
            for (const auto& group : object.groups) {
                for (const auto& face : group.faces) {
                    // Para cada face, processar suas arestas
                    for (size_t i = 0; i < face.vertices.size(); ++i) {
                        size_t v1_idx = face.vertices[i].v - 1;
                        size_t v2_idx = face.vertices[(i + 1) % face.vertices.size()].v - 1;
                        
                        if (v1_idx >= vertices.size() || v2_idx >= vertices.size()) continue;
                        
                        // Garantir ordem consistente para evitar duplicatas
                        std::size_t min_idx = std::min(v1_idx, v2_idx);
                        std::size_t max_idx = std::max(v1_idx, v2_idx);
                        
                        if (edgeSet.find({min_idx, max_idx}) == edgeSet.end()) {
                            edgeSet.insert({min_idx, max_idx});
                            edges.emplace_back(vertices[v1_idx], vertices[v2_idx]);
                        }
                    }
                }
            }
        }
        
        return edges;
    }

    /**
     * @brief Retorna todas as arestas com seus índices.
     * @return Vetor de pares de índices (arestas)
     */
    std::vector<std::pair<std::size_t, std::size_t>> getAllEdgeIndices() const {
        std::vector<std::pair<std::size_t, std::size_t>> edges;
        std::set<std::pair<std::size_t, std::size_t>> edgeSet;
        
        for (const auto& object : objects) {
            for (const auto& group : object.groups) {
                for (const auto& face : group.faces) {
                    for (size_t i = 0; i < face.vertices.size(); ++i) {
                        size_t v1_idx = face.vertices[i].v - 1;
                        size_t v2_idx = face.vertices[(i + 1) % face.vertices.size()].v - 1;
                        
                        if (v1_idx >= vertices.size() || v2_idx >= vertices.size()) continue;
                        
                        std::size_t min_idx = std::min(v1_idx, v2_idx);
                        std::size_t max_idx = std::max(v1_idx, v2_idx);
                        
                        if (edgeSet.find({min_idx, max_idx}) == edgeSet.end()) {
                            edgeSet.insert({min_idx, max_idx});
                            edges.emplace_back(v1_idx, v2_idx);
                        }
                    }
                }
            }
        }
        
        return edges;
    }

    /**
     * @brief Retorna todas as faces do modelo como triângulos.
     * @return Vetor de tuplas com índices dos vértices de cada triângulo
     */
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> getAllFacesAsIndices() const {
        std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> triangles;
        
        for (const auto& object : objects) {
            for (const auto& group : object.groups) {
                for (const auto& face : group.faces) {
                    if (face.vertices.size() >= 3) {
                        // Triangulação simples (fan triangulation)
                        for (size_t i = 1; i + 1 < face.vertices.size(); ++i) {
                            size_t i0 = face.vertices[0].v - 1;
                            size_t i1 = face.vertices[i].v - 1;
                            size_t i2 = face.vertices[i + 1].v - 1;
                            
                            if (i0 < vertices.size() && i1 < vertices.size() && i2 < vertices.size()) {
                                triangles.emplace_back(i0, i1, i2);
                            }
                        }
                    }
                }
            }
        }
        
        return triangles;
    }

    /**
     * @brief Retorna os vértices como pontos float para renderização.
     */
    std::vector<std::array<float, 3>> getVerticesAsFloat() const
        requires (std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        std::vector<std::array<float, 3>> float_vertices;
        for (const auto& v : vertices) {
            float_vertices.push_back({static_cast<float>(v[0]), 
                                    static_cast<float>(v[1]), 
                                    static_cast<float>(v[2])});
        }
        return float_vertices;
    }

    /**
     * @brief Retorna as arestas como pares de pontos float para renderização.
     */
    std::vector<std::pair<std::array<float, 3>, std::array<float, 3>>> getEdgesAsFloat() const
        requires (std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        auto edges = getAllEdgeIndices();
        std::vector<std::pair<std::array<float, 3>, std::array<float, 3>>> float_edges;
        
        for (const auto& edge : edges) {
            const auto& v1 = vertices[edge.first];
            const auto& v2 = vertices[edge.second];
            
            float_edges.push_back({
                {static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2])},
                {static_cast<float>(v2[0]), static_cast<float>(v2[1]), static_cast<float>(v2[2])}
            });
        }
        
        return float_edges;
    }

private:
    static VertexIndices parse_vertex_indices(const std::string& token) {
        VertexIndices vi;
        std::vector<std::string> parts;
        std::stringstream ss(token);
        std::string part;
        
        while (std::getline(ss, part, '/')) {
            parts.push_back(part);
        }

        if (parts.size() >= 1 && !parts[0].empty()) vi.v = std::stoll(parts[0]);
        if (parts.size() >= 2 && !parts[1].empty()) vi.vt = std::stoll(parts[1]);
        if (parts.size() >= 3 && !parts[2].empty()) vi.vn = std::stoll(parts[2]);
        
        return vi;
    }
};