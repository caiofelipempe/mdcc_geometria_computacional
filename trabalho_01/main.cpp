#include "renderer.hpp"
#include "input.h"
#include "utils/file_saver.hpp"
#include "utils/file_selector.hpp"
#include "utils/convex_hull.hpp"
#include "utils/obj_model.hpp"
#include "vector.hpp"
#include "point.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <optional>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

using namespace geometry;

class Trabalho01 : public Renderer {
public:
    Trabalho01() = default;

protected:
    void onInit(int w, int h, const std::string&) override {
        onWindowResize(w, h);
    }

    void onWindowResize(int w, int h) override {
        width  = w;
        height = h;
    }

    void onUpdate(float dt) override {
        this->dt = dt;
    }

    void onUI() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin(
            "Main",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        float fullHeight = ImGui::GetContentRegionAvail().y;

        ImGui::BeginChild("Panel", ImVec2(leftPanelWidth, fullHeight), true);
        panelUI();
        ImGui::EndChild();

        drawVerticalSplitter(leftPanelWidth, 150.0f, 200.0f);

        ImGui::SameLine();
        ImGui::BeginChild(
            "Canvas",
            ImVec2(0, fullHeight),
            true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );
        drawCanvas();
        ImGui::EndChild();

        ImGui::End();
    }

private:
    int width  = 0;
    int height = 0;

    float dt;

    float leftPanelWidth = 300.0f;

    FileSaver fileSaver;
    FileSelector fileSelector;

    std::vector<std::tuple<std::string, std::vector<Point3f>>> objectPoints;
    std::optional<ObjModel<float, 3>> objModel = std::nullopt;
    bool showOriginalPoints = false;
    bool showVertices = false;
    bool showEdges = false;
    bool showFaces = true;

    GLuint fbo = 0;
    GLuint fboTexture = 0;
    GLuint rbo = 0;
    ImVec2 canvasSize = {0, 0};

private:
    std::vector<std::tuple<std::string, std::vector<Point3f>>> objectPointsFromOBJ(const std::string& conteudo) {
        std::vector<std::tuple<std::string, std::vector<Point3f>>> objetos;
        
        std::istringstream stream(conteudo);
        std::string linha;
        std::string objetoAtual = "default";
        std::vector<Point3f> verticesAtuais;
        bool primeiroObjeto = true;
        
        auto finalizarObjetoAtual = [&]() {
            if (!verticesAtuais.empty() || !primeiroObjeto) {
                objetos.emplace_back(objetoAtual, verticesAtuais);
                verticesAtuais.clear();
            }
            primeiroObjeto = false;
        };
        
        while (std::getline(stream, linha)) {
            linha.erase(0, linha.find_first_not_of(" \t\r\n"));
            
            if (linha.empty() || linha[0] == '#') continue;
            
            if (linha[0] == 'o' || linha[0] == 'g') {
                finalizarObjetoAtual();
                
                std::istringstream ss(linha.substr(1));
                ss >> objetoAtual;
                if (objetoAtual.empty()) {
                    objetoAtual = "objeto_" + std::to_string(objetos.size() + 1);
                }
            }
            else if (linha.size() >= 2 && linha[0] == 'v' && 
                    (linha[1] == ' ' || linha[1] == '\t')) {
                
                std::istringstream ss(linha.substr(1));
                float x, y, z;
                if (ss >> x >> y >> z) {
                    verticesAtuais.push_back(Point3f({x, y, z}));
                }
            }
        }
        
        if (!verticesAtuais.empty() || objetos.empty()) {
            objetos.emplace_back(objetoAtual, verticesAtuais);
        }
        
        return objetos;
    }

    void drawVerticalSplitter(float& leftWidth, float minLeft, float minRight) {
        ImGui::SameLine();

        ImGui::InvisibleButton("##splitter", ImVec2(6.0f, -1.0f));

        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            leftWidth += delta;
        }

        float total = ImGui::GetContentRegionAvail().x + leftWidth;
        leftWidth = ImClamp(leftWidth, minLeft, total - minRight);
    }

    void panelUI() {
        if (ImGui::Button("Buscar Modelo")) {
            fileSelector.Open();
        }
        
        if (ImGui::Button("Gerar Pontos Randômicos")) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(-5.0, 5.0);

                objectPoints.clear();
                objModel = std::nullopt;
                std::vector<Point3f> points;
                for (int i = 0; i < 100; ++i) {
                    points.push_back(Point3f({(float)dis(gen), (float)dis(gen), (float)dis(gen)}));
                }

                objectPoints.emplace_back("main", points);
        }
        
        if (!fileSelector.GetContent().empty()) {
            objectPoints = objectPointsFromOBJ(fileSelector.GetContent().c_str());
            fileSelector.ClearContent();
            objModel = std::nullopt;
        }
        
        fileSelector.Draw();
        
        if(!objectPoints.empty()) {
            ImGui::Separator();
            if(objModel.has_value()) {
                const auto& model = objModel.value();

                ImGui::Checkbox("Mostrar pontos originais", &showOriginalPoints);
                ImGui::Checkbox("Mostrar vertices", &showVertices);
                ImGui::Checkbox("Mostrar arestas", &showEdges);
                ImGui::Checkbox("Mostrar faces", &showFaces);

                if (ImGui::Button("Salvar Modelo")) {
                    fileSaver.Open();
                }
            } else {
                if (ImGui::Button("Convex Hull")) {
                    std::vector<std::tuple<std::string, geometry::Mesh3f>> meshes;
                    for(auto& [name, points] : objectPoints) {
                        meshes.push_back(std::tuple(name, convex_hull::bruteForce(points)));
                    }
                    objModel = ObjModel<float, 3>::fromMeshes(meshes);
                }
            }
        }

        fileSaver.Draw();
        if (fileSaver.HasSelected() && objModel.has_value()) {
            objModel.value().save(fileSaver.GetSelectedPath());
        }
    }

    void setupFBO(int w, int h) {
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &fboTexture);
            glDeleteRenderbuffers(1, &rbo);
        }

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Erro: FBO incompleto!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderScene(int w, int h) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, w, h);
        
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, (double)w/(double)h, 0.1, 1000.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        static float angle = 0.0f;
        gluLookAt(
            15.0f * std::cos(angle), 10.0f, 15.0f * std::sin(angle), 
            0.0f, 0.0f, 0.0f, 
            0.0f, 1.0f, 0.0f
        );
        angle += dt;

        if(objModel.has_value()) {
            const auto& model = objModel.value();
            
            if(showFaces) {
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
                
                GLfloat light_pos[] = { 5.0f, 10.0f, 5.0f, 1.0f };
                GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
                GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
                GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                
                glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
                glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
                glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
                
                GLfloat material_ambient[] = { 0.7f, 0.5f, 0.3f, 1.0f };
                GLfloat material_diffuse[] = { 0.8f, 0.6f, 0.4f, 1.0f };
                GLfloat material_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
                GLfloat material_shininess[] = { 32.0f };
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
                glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
                
                // Usar as faces do modelo
                auto faces = model.getAllFacesAsIndices();
                const auto& vertices = model.vertices;
                const auto& normals = model.normals;
                
                glBegin(GL_TRIANGLES);
                
                for (const auto& face : faces) {
                    std::size_t i0 = std::get<0>(face);
                    std::size_t i1 = std::get<1>(face);
                    std::size_t i2 = std::get<2>(face);
                    
                    // Tentar usar normals se disponíveis
                    if (i0 < normals.size() && i1 < normals.size() && i2 < normals.size()) {
                        const auto& n0 = normals[i0];
                        const auto& n1 = normals[i1];
                        const auto& n2 = normals[i2];
                        
                        glNormal3f(n0[0], n0[1], n0[2]);
                        glVertex3f(vertices[i0][0], vertices[i0][1], vertices[i0][2]);
                        
                        glNormal3f(n1[0], n1[1], n1[2]);
                        glVertex3f(vertices[i1][0], vertices[i1][1], vertices[i1][2]);
                        
                        glNormal3f(n2[0], n2[1], n2[2]);
                        glVertex3f(vertices[i2][0], vertices[i2][1], vertices[i2][2]);
                    } else {
                        // Calcular normal da face se não houver normals
                        geometry::Vec3f v0({vertices[i0][0], vertices[i0][1], vertices[i0][2]});
                        geometry::Vec3f v1({vertices[i1][0], vertices[i1][1], vertices[i1][2]});
                        geometry::Vec3f v2({vertices[i2][0], vertices[i2][1], vertices[i2][2]});
                        
                        geometry::Vec3f edge1 = v1 - v0;
                        geometry::Vec3f edge2 = v2 - v0;
                        geometry::Vec3f face_normal = edge1.cross3(edge2).normalized();
                        
                        glNormal3f(face_normal[0], face_normal[1], face_normal[2]);
                        glVertex3f(vertices[i0][0], vertices[i0][1], vertices[i0][2]);
                        glVertex3f(vertices[i1][0], vertices[i1][1], vertices[i1][2]);
                        glVertex3f(vertices[i2][0], vertices[i2][1], vertices[i2][2]);
                    }
                }
                
                glEnd();
                
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHTING);
            }
            
            if(showEdges) {
                glDisable(GL_LIGHTING);
                glLineWidth(1.f);
                glColor3f(0.0f, 1.0f, 0.0f);
                
                auto edges = model.getAllEdgeIndices();
                const auto& vertices = model.vertices;
                
                glBegin(GL_LINES);
                
                for (const auto& edge : edges) {
                    const auto& v1 = vertices[edge.first];
                    const auto& v2 = vertices[edge.second];
                    
                    glVertex3f(v1[0], v1[1], v1[2]);
                    glVertex3f(v2[0], v2[1], v2[2]);
                }
                
                glEnd();
            }
            
            if(showVertices) {
                glDisable(GL_LIGHTING);
                glPointSize(2.f);
                glColor3f(1.0f, 0.0f, 0.0f);
                
                const auto& vertices = model.vertices;
                
                glBegin(GL_POINTS);
                
                for (const auto& vertex : vertices) {
                    glVertex3f(vertex[0], vertex[1], vertex[2]);
                }
                
                glEnd();
            }
            
            if(showOriginalPoints) {
                glDisable(GL_LIGHTING);
                glPointSize(2.f);
                glColor3f(1.0f, 1.0f, 0.0f);
                
                const auto& vertices = model.vertices;
                
                glBegin(GL_POINTS);
                
                for (const auto& [name, points] : objectPoints) {
                    for(const auto& point : points) {
                        glVertex3f(point[0], point[1], point[2]);
                    }
                }
                
                glEnd();
            }
            
        } else if (!objectPoints.empty()) {
            glDisable(GL_LIGHTING);
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.8f, 0.0f);
            for (auto& [name, points] : objectPoints) {
                for (auto& p : points) {
                    glVertex3f(p[0], p[1], p[2]);
                }
            }
            glEnd();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glPopAttrib();
    }

    void drawCanvas() {
        ImVec2 currentSize = ImGui::GetContentRegionAvail();

        if (currentSize.x < 1.0f || currentSize.y < 1.0f) return;

        if (currentSize.x != canvasSize.x || currentSize.y != canvasSize.y) {
            canvasSize = currentSize;
            setupFBO((int)canvasSize.x, (int)canvasSize.y);
        }

        renderScene((int)canvasSize.x, (int)canvasSize.y);

        ImGui::Image(
            (ImTextureID)(intptr_t)fboTexture, 
            canvasSize, 
            ImVec2(0, 1), 
            ImVec2(1, 0)
        );
    }
};

int main() {
    Trabalho01 app;
    app.run(800, 600, "Tarefa 02 - Geometria Computacional");
}