#include "renderer.hpp"
#include "input.h"
#include "vector.hpp"
#include "utils.hpp"

#include <imgui.h>
#include <imgui_internal.h>

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

private:
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
        ImGui::TextUnformatted("Painel Esquerdo");
        ImGui::Separator();
        ImGui::TextUnformatted("Controles aqui");
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
