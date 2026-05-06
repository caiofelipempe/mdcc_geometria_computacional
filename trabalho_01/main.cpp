#include "renderer.hpp"
#include "input.h"
#include "utils/file_selector.hpp"
#include "vector.hpp"
#include "point.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>

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

    void drawCanvas() {
        ImGui::TextUnformatted("Canvas");
        ImGui::Separator();

        ImVec2 pos  = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();

        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRectFilled(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(45, 45, 45, 255)
        );

        dl->AddCircle(
            ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f),
            50.0f,
            IM_COL32(220, 100, 100, 255),
            32,
            3.0f
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
