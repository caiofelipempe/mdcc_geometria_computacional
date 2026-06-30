#include "renderer.hpp"
#include "input.h"
#include "utils/file_saver.hpp"
#include "utils/file_selector.hpp"
#include "utils/tetrahedralization.hpp"
#include "vector.hpp"
#include "point.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <optional>
#include <random>
#include <future>
#include <mutex>

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
        {
            std::scoped_lock lock(meshesMutex);

            updateButtomClick();
            updateFileSelector();
            updateFileSaver();
            updateInput();
            updateCamera();
        }

        if(delaunayStop) {
            if(delaunayFut.valid()) {
                delaunayFut.wait();
            }

            delaunayStop = false;
            meshes.clear();
        }
    }

    void onUI() override {
        std::unique_lock<std::mutex> lock_principal(meshesMutex);

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
        
        meshesMutex.unlock();
    }

private:
    int width  = 0;
    int height = 0;

    float dt;

    double mouseX{0.0};
    double mouseY{0.0};
    double mouseDx{0.0};
    double mouseDy{0.0};

    float leftPanelWidth = 300.0f;

    FileSaver fileSaver;
    FileSelector fileSelector;

    enum ButtonClick {
        none = 0,
        loadModel,
        generateRandomPoints,
        generateCubePoints,
        generateSpherePoints,
        generateCylinderPoints,
        generateConePoints,
        resetCamera,
        clearPoints,
        clearModel,
        saveModel,
        cgal,
        delaunay,
        delaunay_rnd,
    } buttonClick = ButtonClick::none;

    int numberOfRandomPoints = 50;
    std::vector<std::tuple<std::string, std::vector<Point3f>>> objectPoints;
    std::vector<std::tuple<std::string, geometry::Mesh3f>> meshes;
    std::mutex meshesMutex;
    std::future<void> delaunayFut;
    std::atomic<bool> delaunayStop = false;
    std::atomic<bool> delaunayRunning = false;
    std::atomic<useconds_t> step_time = 3000;
    bool showPoints = true;
    bool showEdges = true;
    bool shouldUpdateCamera = true;

    GLuint fbo = 0;
    GLuint fboTexture = 0;
    GLuint rbo = 0;
    ImVec2 canvasSize = {0, 0};
    ImVec2 canvasOrigin = {0, 0};
    bool holdingOnCanvasMouseBtn0 = false;
    bool holdingOnCanvasMouseBtn1 = false;

    struct LookAt {
        float boom = 15;
        float angleX = 0;
        float angleY = 0;
        GLdouble centerX = 0;
        GLdouble centerY = 0;
        GLdouble centerZ = 0;
        GLdouble forwardX = 0;
        GLdouble forwardY = 0;
        GLdouble forwardZ = 0;
        GLdouble upX = 0;
        GLdouble upY = 0;
        GLdouble upZ = 0;
        GLdouble rightX = 0;
        GLdouble rightY = 0;
        GLdouble rightZ = 0;
    } camera;

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

    std::string objectMeshesToOBJ(
        const std::vector<std::tuple<std::string, geometry::Mesh3f>>& objetos)
    {
        std::ostringstream out;

        out << "# Generated OBJ\n";

        std::size_t vertex_offset = 1;

        for (const auto& [nome, mesh] : objetos) {
            const auto& vertices = mesh.getVertices();
            const auto& faces    = mesh.getFaces();
            const auto& tets     = mesh.getTetrahedrons();

            if (vertices.empty()) continue;

            out << "o " << nome << "\n";

            for (const auto& v : vertices) {
                out << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
            }

            for (const auto& f : faces) {
                out << "f "
                    << (vertex_offset + f[0]) << " "
                    << (vertex_offset + f[1]) << " "
                    << (vertex_offset + f[2]) << "\n";
            }

            for (const auto& t : tets) {
                std::size_t i0 = vertex_offset + t[0];
                std::size_t i1 = vertex_offset + t[1];
                std::size_t i2 = vertex_offset + t[2];
                std::size_t i3 = vertex_offset + t[3];

                out << "f " << i0 << " " << i1 << " " << i2 << "\n";
                out << "f " << i0 << " " << i1 << " " << i3 << "\n";
                out << "f " << i0 << " " << i2 << " " << i3 << "\n";
                out << "f " << i1 << " " << i2 << " " << i3 << "\n";
            }

            vertex_offset += vertices.size();
        }

        return out.str();
    }

    void updateButtomClick() {
        if(buttonClick != ButtonClick::none && delaunayRunning) {
            delaunayStop = true;
            return;
        }

        switch (buttonClick)
        {
        case ButtonClick::loadModel:
            {
                fileSelector.Open();
            }
            break;
        
        case ButtonClick::generateRandomPoints:
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(-1.0, 1.0);

                objectPoints.clear();
                meshes.clear();
                std::vector<Point3f> points;
                for (int i = 0; i < numberOfRandomPoints; ++i) {
                    if(delaunayStop) return;
                    points.push_back(Point3f({(float)dis(gen), (float)dis(gen), (float)dis(gen)}));
                }

                objectPoints.emplace_back("main", points);
            }
            break;
        
        case ButtonClick::generateCubePoints:
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(-1.0, 1.0);

                objectPoints.clear();
                meshes.clear();

                std::vector<Point3f> points;
                points.push_back(Point3f({1.f, 1.f, 1.f}));
                points.push_back(Point3f({1.f, 1.f, -1.f}));
                points.push_back(Point3f({1.f, -1.f, 1.f}));
                points.push_back(Point3f({1.f, -1.f, -1.f}));
                points.push_back(Point3f({-1.f, 1.f, 1.f}));
                points.push_back(Point3f({-1.f, 1.f, -1.f}));
                points.push_back(Point3f({-1.f, -1.f, 1.f}));
                points.push_back(Point3f({-1.f, -1.f, -1.f}));

                for (int i = 0; i < numberOfRandomPoints; ++i) {
                    points.push_back(Point3f({(float)dis(gen), (float)dis(gen), (float)dis(gen)}));
                }

                objectPoints.emplace_back("main", points);
            }
            break;

        case ButtonClick::generateSpherePoints:
            {
                objectPoints.clear();
                meshes.clear();
                std::vector<Point3f> points;

                float radius = 2.0f;
                int rings = 12;
                int sectors = 12;

                points.push_back(Point3f({0.f, radius, 0.f}));
                points.push_back(Point3f({0.f, -radius, 0.f}));

                for (int r = 1; r < rings; ++r) {
                    float phi = M_PI * (float)r / (float)rings;
                    float y = radius * std::cos(phi);
                    float ringRadius = radius * std::sin(phi);

                    for (int s = 0; s < sectors; ++s) {
                        float theta = 2.0f * M_PI * (float)s / (float)sectors;
                        float x = ringRadius * std::cos(theta);
                        float z = ringRadius * std::sin(theta);

                        points.push_back(Point3f({x, y, z}));
                    }
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> disReal(-radius, radius);

                for (int i = 0; i < numberOfRandomPoints; ++i) {
                    float x = (float)disReal(gen);
                    float y = (float)disReal(gen);
                    float z = (float)disReal(gen);
                    
                    if (x*x + y*y + z*z <= radius*radius) {
                        points.push_back(Point3f({x, y, z}));
                    } else {
                        --i;
                    }
                }

                objectPoints.emplace_back("sphere", points);
            }
            break;

        case ButtonClick::generateCylinderPoints:
            {
                objectPoints.clear();
                meshes.clear();
                std::vector<Point3f> points;

                float radius = 1.5f;
                float height = 4.0f;
                int slices = 16;
                int stacks = 6;

                for (int i = 0; i <= stacks; ++i) {
                    float y = -height / 2.0f + height * ((float)i / (float)stacks);

                    for (int j = 0; j < slices; ++j) {
                        float theta = 2.0f * M_PI * (float)j / (float)slices;
                        float x = radius * std::cos(theta);
                        float z = radius * std::sin(theta);

                        points.push_back(Point3f({x, y, z}));
                    }
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> disRadius(0.0, radius);
                std::uniform_real_distribution<> disAngle(0.0, 2.0 * M_PI);
                std::uniform_real_distribution<> disHeight(-height / 2.0f, height / 2.0f);

                for (int i = 0; i < numberOfRandomPoints; ++i) {
                    float r = (float)disRadius(gen);
                    float theta = (float)disAngle(gen);
                    
                    float x = r * std::cos(theta);
                    float z = r * std::sin(theta);
                    float y = (float)disHeight(gen);

                    points.push_back(Point3f({x, y, z}));
                }

                objectPoints.emplace_back("cylinder", points);
            }
            break;

            case ButtonClick::generateConePoints:
            {
                objectPoints.clear();
                meshes.clear();
                std::vector<Point3f> points;

                float radius = 2.0f;
                float height = 4.0f;
                int slices = 16;
                int stacks = 5;

                float halfHeight = height / 2.0f;
                Point3f apex({0.f, halfHeight, 0.f});

                points.push_back(apex);

                points.push_back(Point3f({0.f, -halfHeight, 0.f}));

                for (int i = 1; i <= stacks; ++i) {
                    float t = (float)i / (float)stacks;
                    float y = halfHeight - t * height;
                    float currentRadius = t * radius;

                    for (int j = 0; j < slices; ++j) {
                        float theta = 2.0f * M_PI * (float)j / (float)slices;
                        float x = currentRadius * std::cos(theta);
                        float z = currentRadius * std::sin(theta);

                        points.push_back(Point3f({x, y, z}));
                    }
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> disT(0.0, 1.0);
                std::uniform_real_distribution<> disR(0.0, 1.0);
                std::uniform_real_distribution<> disAngle(0.0, 2.0 * M_PI);

                for (int i = 0; i < numberOfRandomPoints; ++i) {
                    float t = (float)disT(gen);
                    float y = halfHeight - t * height;
                    
                    float maxRadiusAtY = t * radius;
                    
                    float r = std::sqrt((float)disR(gen)) * maxRadiusAtY;
                    float theta = (float)disAngle(gen);

                    float x = r * std::cos(theta);
                    float z = r * std::sin(theta);

                    points.push_back(Point3f({x, y, z}));
                }

                objectPoints.emplace_back("cone", points);
            }
            break;
        
        case ButtonClick::resetCamera:
            {
                    camera.centerX = 0; camera.centerY = 0; camera.centerZ = 0;
                    camera.angleX = 0; camera.angleY = 0; camera.boom = 15;
                    
                    shouldUpdateCamera = true;
            }
            break;
        
        case ButtonClick::clearPoints:
            {
                objectPoints.clear();
                meshes.clear();
            }
            break;
        
        case ButtonClick::clearModel:
            {
                meshes.clear();
            }
            break;
        
        case ButtonClick::saveModel:
            {
                fileSaver.Open();
            }
            break;
        
        case ButtonClick::cgal:
            {
                meshes.clear();

                for (auto& [name, points] : objectPoints) {
                    geometry::Mesh3f mesh;
                    for (const auto& p : points) {
                        auto _ = mesh.addVertex(p);
                    }
                    meshes.emplace_back(name, mesh);
                }
                for (auto &[_, mesh] : meshes) {
                    tetrahedralization::cgal(mesh);
                }
            }
            break;
        
        case ButtonClick::delaunay:
        case ButtonClick::delaunay_rnd:
            {
                meshes.clear();

                for (auto& [name, points] : objectPoints) {
                    geometry::Mesh3f mesh;
                    for (const auto& p : points) {
                        auto _ = mesh.addVertex(p);
                    }
                    meshes.emplace_back(name, mesh);
                }

                delaunayRunning = true;
                bool rnd = buttonClick == ButtonClick::delaunay_rnd;
                delaunayFut = std::async(std::launch::async, ([this, rnd]() {
                    if(delaunayStop) return;
                    for (auto &[_, mesh] : meshes) {
                        tetrahedralization::delaunay(mesh, step_time, meshesMutex, delaunayStop, rnd);
                    }

                    delaunayRunning = false;
                }));
            }
            break;
        
        default:
            break;
        }

        buttonClick = ButtonClick::none;
    }

    void updateFileSelector() {
        if (!fileSelector.GetContent().empty()) {
            objectPoints = objectPointsFromOBJ(fileSelector.GetContent().c_str());
            fileSelector.ClearContent();
            meshes.clear();
        }
    }

    void updateFileSaver() {
        if (fileSaver.HasSelected() && meshes.size() > 0) {
            auto path = fileSaver.GetSelectedPath();

            std::string objData = objectMeshesToOBJ(meshes);

            if (path.size() < 4 || path.substr(path.size() - 4) != ".obj") {
                path += ".obj";
            }

            std::ofstream file(path);
            if (file.is_open()) {
                file << objData;
                file.close();
            } else {
                std::cerr << "Erro ao salvar arquivo: " << path << "\n";
            }
        }
    }

    void updateInput() {
        auto& input = this->input();
        this->dt = dt;

        mouseDx = input.mouseX - mouseX;
        mouseDy = input.mouseY - mouseY;
        mouseX = input.mouseX;
        mouseY = input.mouseY;

        if (isOncanvas(input.mouseX, input.mouseY)) {
            float moveSpeed = 5.0f * dt * (camera.boom * 0.1f + 1.0f);

            if (input.keys[GLFW_KEY_W]) {
                camera.centerX -= camera.forwardX * moveSpeed;
                camera.centerY -= camera.forwardY * moveSpeed;
                camera.centerZ -= camera.forwardZ * moveSpeed;
                shouldUpdateCamera = true;
            }
            if (input.keys[GLFW_KEY_S]) {
                camera.centerX += camera.forwardX * moveSpeed;
                camera.centerY += camera.forwardY * moveSpeed;
                camera.centerZ += camera.forwardZ * moveSpeed;
                shouldUpdateCamera = true;
            }
            if (input.keys[GLFW_KEY_A]) {
                camera.centerX -= camera.rightX * moveSpeed;
                camera.centerY -= camera.rightY * moveSpeed;
                camera.centerZ -= camera.rightZ * moveSpeed;
                shouldUpdateCamera = true;
            }
            if (input.keys[GLFW_KEY_D]) {
                camera.centerX += camera.rightX * moveSpeed;
                camera.centerY += camera.rightY * moveSpeed;
                camera.centerZ += camera.rightZ * moveSpeed;
                shouldUpdateCamera = true;
            }

            if (input.scrollOffset != 0) {
                shouldUpdateCamera = true;
                camera.boom -= input.scrollOffset * (camera.boom * 0.1f + 0.2f);
                if (camera.boom < 0.1f) camera.boom = 0.1f;
            }

            holdingOnCanvasMouseBtn0 = input.mouseButtons[0];
            holdingOnCanvasMouseBtn1 = input.mouseButtons[1];

        } else {
            if (!input.mouseButtons[0]) holdingOnCanvasMouseBtn0 = false;
            if (!input.mouseButtons[1]) holdingOnCanvasMouseBtn1 = false;
        }

        if (holdingOnCanvasMouseBtn0 && (mouseDx != 0 || mouseDy != 0)) {
            shouldUpdateCamera = true;
            camera.angleX += mouseDx * (-0.005f);
            camera.angleY = std::clamp(camera.angleY + (float)mouseDy * 0.005f, (float)-M_PI/2.1f, (float)M_PI/2.1f);
        }

        if (holdingOnCanvasMouseBtn1 && (mouseDx != 0 || mouseDy != 0)) {
            shouldUpdateCamera = true;
            float panSpeed = camera.boom * 0.001f;
            camera.centerX += (mouseDx * -panSpeed * camera.rightX + mouseDy * panSpeed * camera.upX);
            camera.centerY += (mouseDx * -panSpeed * camera.rightY + mouseDy * panSpeed * camera.upY);
            camera.centerZ += (mouseDx * -panSpeed * camera.rightZ + mouseDy * panSpeed * camera.upZ);
        }
    }

    void updateCamera() {
        if(shouldUpdateCamera) {
            float cosY = std::cos(camera.angleY);
            float sinY = std::sin(camera.angleY);
            float cosX = std::cos(camera.angleX);
            float sinX = std::sin(camera.angleX);

            camera.forwardX = cosY * sinX;
            camera.forwardY = sinY;
            camera.forwardZ = cosY * cosX;

            camera.upX = -sinY * sinX;
            camera.upY =  cosY;
            camera.upZ = -sinY * cosX;

            camera.rightX = cosX;
            camera.rightY = 0.0f;
            camera.rightZ = -sinX;
        }

        shouldUpdateCamera = false;
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
            buttonClick = ButtonClick::loadModel;
        }

        ImGui::DragInt("Quantidade de Números Randômicos", &numberOfRandomPoints);
        
        if (ImGui::Button("Gerar Pontos Randômicos")) {
            buttonClick = ButtonClick::generateRandomPoints;
        }
        
        if (ImGui::Button("Gerar Cubo")) {
            buttonClick = ButtonClick::generateCubePoints;
        }
        
        if (ImGui::Button("Gerar Esfera")) {
            buttonClick = ButtonClick::generateSpherePoints;
        }
        
        if (ImGui::Button("Gerar Cilindro")) {
            buttonClick = ButtonClick::generateCylinderPoints;
        }
        
        if (ImGui::Button("Gerar Cone")) {
            buttonClick = ButtonClick::generateConePoints;
        }
        
        fileSelector.Draw();
        
        if(!objectPoints.empty()) {
            ImGui::Separator();
            if (ImGui::Button("Limpar Pontos")) {
                buttonClick = ButtonClick::clearPoints;
            }
            ImGui::Separator();
            
            if (ImGui::CollapsingHeader("Controle de Câmera", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::DragFloat("Distância (Boom)", &camera.boom, 0.1f, 0.1f, 1000.0f)) shouldUpdateCamera = true;
                
                if (ImGui::SliderAngle("Ângulo X", &camera.angleX)) shouldUpdateCamera = true;
                if (ImGui::SliderAngle("Ângulo Y", &camera.angleY, -89.0f, 89.0f)) shouldUpdateCamera = true;

                ImGui::Separator();
                ImGui::Text("Centro do Alvo:");
                
                float cX = (float)camera.centerX;
                float cY = (float)camera.centerY;
                float cZ = (float)camera.centerZ;

                if (ImGui::DragFloat("X", &cX, 0.1f)) { camera.centerX = cX; shouldUpdateCamera = true; }
                if (ImGui::DragFloat("Y", &cY, 0.1f)) { camera.centerY = cY; shouldUpdateCamera = true; }
                if (ImGui::DragFloat("Z", &cZ, 0.1f)) { camera.centerZ = cZ; shouldUpdateCamera = true; }

                if (ImGui::Button("Resetar Câmera")) {
                    buttonClick = ButtonClick::resetCamera;
                }
            }
            ImGui::Separator();

            if(meshes.size() > 0) {
                ImGui::Checkbox("Mostrar pontos", &showPoints);
                ImGui::Checkbox("Mostrar arestas", &showEdges);

                if (ImGui::Button("Salvar Modelo")) {
                    buttonClick = ButtonClick::saveModel;
                }
                ImGui::Separator();
                if (ImGui::Button("Limpar Modelo")) {
                    buttonClick = ButtonClick::clearModel;
                }
                ImGui::Separator();
                if(delaunayRunning) {
                    int dragStepTime = step_time;
                    ImGui::DragInt("Tempo dos passos", &dragStepTime);
                    step_time = dragStepTime;
                }
            } else {
                ImGui::Separator();

                ImGui::Text("Tetrahedralização");
                if (ImGui::Button("CGal")) {
                    buttonClick = ButtonClick::cgal;
                }
                int dragStepTime = step_time;
                ImGui::DragInt("Tempo dos passos", &dragStepTime);
                step_time = dragStepTime;
                if (ImGui::Button("Delaunay")) {
                    buttonClick = ButtonClick::delaunay;
                }
                if (ImGui::Button("Delaunay Rnd")) {
                    buttonClick = ButtonClick::delaunay_rnd;
                }

                ImGui::Separator();
            }
        }

        fileSaver.Draw();
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


        gluLookAt(
            camera.centerX + camera.forwardX * camera.boom, 
            camera.centerY + camera.forwardY * camera.boom, 
            camera.centerZ + camera.forwardZ * camera.boom,
            
            camera.centerX, 
            camera.centerY, 
            camera.centerZ, 
            
            camera.upX, 
            camera.upY, 
            camera.upZ
        );

        if(!meshes.empty()) {
            if(showEdges) {
                glDisable(GL_LIGHTING);
                glLineWidth(1.f);
                glColor3f(0.0f, 1.0f, 0.0f);
                
                glBegin(GL_LINES);
                
                for (const auto& [name, mesh] : meshes) {
                    auto vertices = mesh.getVertices();
                    for (const auto &tetrahedron : mesh.getTetrahedrons()) {
                        auto v0 = vertices[tetrahedron[0]];
                        auto v1 = vertices[tetrahedron[1]];
                        auto v2 = vertices[tetrahedron[2]];
                        auto v3 = vertices[tetrahedron[3]];
                        glVertex3f(v0[0], v0[1], v0[2]);
                        glVertex3f(v1[0], v1[1], v1[2]);
                        glVertex3f(v1[0], v1[1], v1[2]);
                        glVertex3f(v2[0], v2[1], v2[2]);
                        glVertex3f(v2[0], v2[1], v2[2]);
                        glVertex3f(v0[0], v0[1], v0[2]);
                        glVertex3f(v3[0], v3[1], v3[2]);
                        glVertex3f(v0[0], v0[1], v0[2]);
                        glVertex3f(v3[0], v3[1], v3[2]);
                        glVertex3f(v1[0], v1[1], v1[2]);
                        glVertex3f(v3[0], v3[1], v3[2]);
                        glVertex3f(v2[0], v2[1], v2[2]);
                    }
                }
                
                glEnd();
            }
            
            if(showPoints) {
                glDisable(GL_LIGHTING);
                glPointSize(2.f);
                glColor3f(1.0f, .0f, .0f);
                
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

        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glPushMatrix();
            glTranslatef(camera.centerX, camera.centerY, camera.centerZ);
            
            float radius = camera.boom * 0.008f; 
            
            drawCompassFresnelSphere(radius, 32, 32, camera);
        glPopMatrix();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glPopAttrib();
    }

    void drawCanvas() {
        ImVec2 currentOrigin = ImGui::GetCursorScreenPos();
        this->canvasOrigin = currentOrigin;

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

    void getColorForNormal(float nx, float ny, float nz, float maxOpacity, float* r, float* g, float* b, float* a) {
        float redIntensity   = std::max(0.0f, ny);
        float greenIntensity = std::max(0.0f, nx);
        float blueIntensity  = std::max(0.0f, nz);
        
        float cyanIntensity    = std::max(0.0f, -ny);
        float magentaIntensity = std::max(0.0f, -nx);
        float yellowIntensity  = std::max(0.0f, -nz);

        *r = redIntensity;
        *g = greenIntensity;
        *b = blueIntensity;
        
        *r += (magentaIntensity * 0.2f + yellowIntensity * 0.2f);
        *g += (cyanIntensity * 0.2f + yellowIntensity * 0.2f);
        *b += (cyanIntensity * 0.2f + magentaIntensity * 0.2f);

        float maxCol = std::max({*r, *g, *b, 1.0f});
        *r /= maxCol; *g /= maxCol; *b /= maxCol;
    }

    void drawCompassFresnelSphere(float radius, int slices, int stacks, const LookAt& camera) {
        float dirX = camera.forwardX;
        float dirY = camera.forwardY;
        float dirZ = camera.forwardZ;

        for (int i = 0; i < stacks; ++i) {
            float lat0 = 3.14159f * (-0.5f + (float)i / stacks);
            float z0 = std::sin(lat0);
            float zr0 = std::cos(lat0);

            float lat1 = 3.14159f * (-0.5f + (float)(i + 1) / stacks);
            float z1 = std::sin(lat1);
            float zr1 = std::cos(lat1);

            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= slices; ++j) {
                float lng = 2.0f * 3.14159f * (float)j / slices;
                float x = std::cos(lng);
                float y = std::sin(lng);

                float nx1 = x * zr1;
                float ny1 = y * zr1;
                float nz1 = z1;
                
                float dot1 = (nx1 * dirX + ny1 * dirY + nz1 * dirZ);
                float alphaFresnel1 = std::pow(1.0f - std::abs(dot1), 2.0f);
                
                float r1, g1, b1, a1;
                float maxAlpha = 0.6f;
                getColorForNormal(nx1, ny1, nz1, maxAlpha, &r1, &g1, &b1, &a1);
                
                glColor4f(r1, g1, b1, alphaFresnel1 * maxAlpha);
                glVertex3f(nx1 * radius, ny1 * radius, nz1 * radius);

                float nx2 = x * zr0;
                float ny2 = y * zr0;
                float nz2 = z0;
                
                float dot2 = (nx2 * dirX + ny2 * dirY + nz2 * dirZ);
                float alphaFresnel2 = std::pow(1.0f - std::abs(dot2), 2.0f);

                float r2, g2, b2, a2;
                getColorForNormal(nx2, ny2, nz2, maxAlpha, &r2, &g2, &b2, &a2);
                
                glColor4f(r2, g2, b2, alphaFresnel2 * maxAlpha);
                glVertex3f(nx2 * radius, ny2 * radius, nz2 * radius);
            }
            glEnd();
        }
    }

    bool isOncanvas(double x, double y) {
        return x > canvasOrigin.x && x < canvasOrigin.x + canvasSize.x && y > canvasOrigin.y&& y < canvasOrigin.y + canvasSize.y;
    }
};

int main() {
    Trabalho01 app;
    app.run(800, 600, "Tarefa 02 - Geometria Computacional");
}