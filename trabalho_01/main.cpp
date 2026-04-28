#include "renderer.hpp"
#include "input.h"
#include "vector.hpp"
#include "utils.hpp"

using namespace geometry;
using namespace sort;

#include <imgui.h>

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
class Trabalho01 : public Renderer {
    int width;
    int height;

protected:
    void onInit(int w, int h, const std::string&) override {
        onWindowResize(w, h);
    }

    void onWindowResize(int w, int h) override {
        width = w;
        height = h;
    }

    void onUpdate(float) override {
        
    }

    void onUI() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::Begin(
            "Main", 
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );
            float fullHeight = height;

            // ---------- Painel Esquerdo ----------
            ImGui::BeginChild("Panel", ImVec2(300, fullHeight), true);
                panelUI();
            ImGui::EndChild();

            // ---------- Splitter ----------
            VerticalSplitter(leftPanelWidth, 150.0f, 200.0f);

            // ---------- Painel Direito (Canvas) ----------
            ImGui::SameLine();
            ImGui::BeginChild("Canvas", ImVec2(0, fullHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                drawCanvas();
            ImGui::EndChild();
        ImGui::End();

    }

private:
    void VerticalSplitter(float& leftWidth, float minLeft, float minRight) {
        ImGui::SameLine();
        ImGui::Button("##splitter", ImVec2(4.0f, -1.0f));
        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            leftWidth += delta;
        }

        float totalWidth = ImGui::GetContentRegionAvail().x + leftWidth;
        leftWidth = ImClamp(leftWidth, minLeft, totalWidth - minRight);
    }

    void panelUI() {
        ImGui::Text("Painel Esquerdo");
        ImGui::Separator();
        ImGui::Text("Controles aqui");
    }

    void drawCanvas() {
        ImGui::Text("Canvas");
        ImGui::Separator();

        ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(
            canvasPos,
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
            IM_COL32(50, 50, 50, 255)
        );

        drawList->AddCircle(
            ImVec2(canvasPos.x + canvasSize.x * 0.5f,
                canvasPos.y + canvasSize.y * 0.5f),
            50.0f,
            IM_COL32(255, 100, 100, 255),
            32,
            3.0f
        );
    }

}

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho01 app;
    app.run(800, 600, "Trabalho 01 - Geometria Computacional");
}
