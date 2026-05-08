#include "renderer.hpp"
#include "input.h"
#include "utils/file_selector.hpp"
#include "vector.hpp"
#include "point.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

using namespace geometry;

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
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

    void onUpdate(float) override {}

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

        // ---------- Painel Esquerdo ----------
        ImGui::BeginChild("Panel", ImVec2(leftPanelWidth, fullHeight), true);
        panelUI();
        ImGui::EndChild();

        // ---------- Splitter ----------
        drawVerticalSplitter(leftPanelWidth, 150.0f, 200.0f);

        // ---------- Painel Direito (Canvas) ----------
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

    float leftPanelWidth = 300.0f;

    FileSelector fileSelector;

    std::vector<std::tuple<std::string, std::vector<Point3f>>> objectPoints;

    GLuint fbo = 0;
    GLuint fboTexture = 0;
    GLuint rbo = 0; // Para o Depth Buffer (necessário em 3D)
    ImVec2 canvasSize = {0, 0};

private:
    std::vector<std::tuple<std::string, std::vector<Point3f>>> objectPointsFromOBJ(const std::string& conteudo) {
        std::vector<std::tuple<std::string, std::vector<Point3f>>> objetos;
        
        std::istringstream stream(conteudo);  // Usa stringstream em vez de ifstream
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
        
        while (std::getline(stream, linha)) {  // Lê do stringstream
            // Remove espaços extras
            linha.erase(0, linha.find_first_not_of(" \t\r\n"));
            
            if (linha.empty() || linha[0] == '#') continue;
            
            if (linha[0] == 'o' || linha[0] == 'g') {
                // Finaliza objeto anterior
                finalizarObjetoAtual();
                
                // Pega nome do novo objeto
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
        
        // Finaliza último objeto
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
        if (ImGui::Button("Abrir Seletor")) {
            fileSelector.Open();
        }
        
        // Mostrar conteúdo do arquivo
        if (!fileSelector.GetContent().empty()) {
            // Usar TextWrapped para texto longo
            objectPoints = objectPointsFromOBJ(fileSelector.GetContent().c_str());
            fileSelector.ClearContent();
        }
        
        fileSelector.Draw();
        
        if(!objectPoints.empty()) {
            ImGui::Separator();
            ImGui::Text("Objetos:");
            ImGui::Separator();
            for (const auto& [group_name, points] : objectPoints) {
                // Exibe o nome do grupo em negrito
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[%s]", group_name.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("(%zu pontos)", points.size());
                
                // Exibe cada ponto
                for (size_t i = 0; i < points.size(); ++i) {
                    const auto& p = points[i];
                    ImGui::TextWrapped("  [%zu] (%.2f, %.2f, %.2f)", i, p[0], p[1], p[2]);
                }
                
                ImGui::Separator();
            }
        }
    }

    void renderScene(int w, int h) {
        // Salva os estados do OpenGL para não quebrar o ImGui
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, w, h);
        
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Cinza levemente diferente
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Configuração de Câmera
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, (double)w/(double)h, 0.1, 1000.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Afastamos a câmera em 15 unidades no eixo Z para ver o centro
        gluLookAt(10, 10, 10,  // Posição da Câmera (olhando de diagonal)
                0, 0, 0,    // Para onde olha (Origem)
                0, 1, 0);   // Vetor "Cima"

        // 1. Desenhar Eixos (X=Vermelho, Y=Verde, Z=Azul)
        glLineWidth(2.0f);
        glBegin(GL_LINES);
            glColor3f(1, 0, 0); glVertex3f(0,0,0); glVertex3f(5,0,0);
            glColor3f(0, 1, 0); glVertex3f(0,0,0); glVertex3f(0,5,0);
            glColor3f(0, 0, 1); glVertex3f(0,0,0); glVertex3f(0,0,5);
        glEnd();

        // 2. Desenhar um Cubo de Arame (Wireframe) para referência
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP); // Face de cima
            glVertex3f(-1, 1, -1); glVertex3f(1, 1, -1);
            glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);
        glEnd();
        glBegin(GL_LINE_LOOP); // Face de baixo
            glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1);
            glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1);
        glEnd();

        // 3. Desenhar os pontos do OBJ (se existirem)
        if (!objectPoints.empty()) {
            glPointSize(5.0f);
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.8f, 0.0f); // Dourado
            for (auto& [name, points] : objectPoints) {
                for (auto& p : points) {
                    glVertex3f(p[0], p[1], p[2]);
                }
            }
            glEnd();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glPopAttrib(); // Restaura os estados do ImGui
    }

    void setupFBO(int w, int h) {
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &fboTexture);
            glDeleteRenderbuffers(1, &rbo);
        }

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Textura de Cor
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        // Buffer de Profundidade (Depth)
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Erro: FBO incompleto!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void drawCanvas() {
        ImVec2 currentSize = ImGui::GetContentRegionAvail();

        if (currentSize.x < 1.0f || currentSize.y < 1.0f) return;

        if (currentSize.x != canvasSize.x || currentSize.y != canvasSize.y) {
            canvasSize = currentSize;
            setupFBO((int)canvasSize.x, (int)canvasSize.y);
        }

        // Renderiza a cena 3D para o FBO
        renderScene((int)canvasSize.x, (int)canvasSize.y);

        // Desenha a textura resultante no ImGui
        // Nota: UV (0,1) e (1,0) para inverter o eixo Y do OpenGL
        ImGui::Image(
            (ImTextureID)(intptr_t)fboTexture, 
            canvasSize, 
            ImVec2(0, 1), 
            ImVec2(1, 0)
        );
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho01 app;
    app.run(800, 600, "Tarefa 02 - Geometria Computacional");
}
