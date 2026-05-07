#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "point.hpp"
#include "vector.hpp"

namespace geometry {

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

template <Arithmetic T, std::size_t N>
class ObjModel {
public:
    // Dados globais do arquivo
    std::vector<Point<T, N>> vertices;
    std::vector<Point<T, 2>> tex_coords; // vt (sempre 2D)
    std::vector<Vector<T, N>> normals;   // vn

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
                Point<T, N> p;
                for (std::size_t i = 0; i < (N == 0 ? 3 : N); ++i) ss >> p[i];
                model.vertices.push_back(p);
            }
            else if (prefix == "vt") {
                Point<T, 2> vt;
                ss >> vt[0] >> vt[1];
                model.tex_coords.push_back(vt);
            }
            else if (prefix == "vn") {
                Vector<T, N> vn;
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

} // namespace geometry