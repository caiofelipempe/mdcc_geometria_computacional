#include "renderer.hpp"
#include "input.h"
#include "vecn.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <random>
#include <cmath>

using Vec2 = geometry::VecN<float, 2>;

class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

protected:
    // -----------------------------------
    // Estado geral
    // -----------------------------------
    int m_width  = 0;
    int m_height = 0;

    float m_panelWidth       = 250.0f;
    const float m_splitterW  = 6.0f;
    const float m_minPanelW  = 120.0f;
    const float m_minCanvasW = 120.0f;

    Questoes m_questao = QUESTAO_1;
    int m_questaoComboIndex = 0;

    // -----------------------------------
    // Estados das questões
    // -----------------------------------
    
    struct Questao1State {
        Vec2 vecA {0.5f, 0.2f};
        Vec2 vecB {0.2f, 0.7f};
    } m_q1;
    
    struct Questao2State {
        Vec2 vec {0.7f, 0.3f};
    } m_q2;

    struct Questao3State {
        int operation = 0;
        Vec2 vecA {0.4f, 0.1f};
        Vec2 vecB {0.2f, 0.6f};
        Vec2 resultVec {};
        float resultScalar = 0.0f;
        static constexpr const char* Q3_OPS[] = {
            "Soma", "Subtração", "Produto Vetorial", "Produto Escalar"
        };
    } m_q3;

    static constexpr const char* QUESTOES_LABELS[] = {
        "Questão 1", "Questão 2", "Questão 3",
        "Questão 4", "Questão 5", "Questão 6"
    };

    // -----------------------------------
    // Ciclo de vida
    // -----------------------------------
    void onInit(int w, int h, const std::string&) override {
        m_width  = w;
        m_height = h;
    }

    void onWindowResize(int w, int h) override {
        m_width  = w;
        m_height = h;
        glViewport(0, 0, w, h);
    }
    
    void onUpdate() override {
        ImGui::Begin("Editor");
        drawLayout();
        ImGui::End();
    }

    // -----------------------------------
    // Layout
    // -----------------------------------
    void drawLayout() {
        ImVec2 avail = ImGui::GetContentRegionAvail();

        m_panelWidth = ImClamp(
            m_panelWidth,
            m_minPanelW,
            avail.x - m_minCanvasW - m_splitterW
        );

        drawLeftPanel(avail);
        drawSplitter(avail);
        drawCanvas(avail);
    }

    // -----------------------------------
    // Painel esquerdo
    // -----------------------------------
    void drawLeftPanel(const ImVec2& avail) {
        ImGui::BeginChild("LeftPanel", {m_panelWidth, avail.y}, true);

        if (ImGui::Combo("Questão", &m_questaoComboIndex,
                         QUESTOES_LABELS, IM_ARRAYSIZE(QUESTOES_LABELS))) {
            m_questao = static_cast<Questoes>(m_questaoComboIndex + 1);
        }

        ImGui::Separator();

        switch (m_questao) {
            case QUESTAO_1: drawQ1Panel(); break;
            case QUESTAO_2: drawQ2Panel(); break;
            case QUESTAO_3: drawQ3Panel(); break;
            default: ImGui::Text("Questão não implementada."); break;
        }

        ImGui::EndChild();
    }

    // -----------------------------------
    // Painéis por questão
    // -----------------------------------

    void drawQ1Panel() {
        ImGui::Text("Questão 1");
        ImGui::DragFloat2("Vetor A", &m_q1.vecA[0], 0.01f, -1.0f, 1.0f);
        ImGui::DragFloat2("Vetor B", &m_q1.vecB[0], 0.01f, -1.0f, 1.0f);
    }

    void drawQ2Panel() {
        ImGui::Text("Questão 2");
        ImGui::DragFloat2("Vetor", &m_q2.vec[0], 0.01f, -1.0f, 1.0f);
    }

    void drawQ3Panel() {
        ImGui::Text("Questão 3");

        ImGui::Combo(
            "Operação",
            &m_q3.operation,
            m_q3.Q3_OPS,
            IM_ARRAYSIZE(m_q3.Q3_OPS)
        );

        ImGui::DragFloat2("Vetor A", &m_q3.vecA[0], 0.01f, -1.0f, 1.0f);
        ImGui::DragFloat2("Vetor B", &m_q3.vecB[0], 0.01f, -1.0f, 1.0f);

        if (ImGui::Button("Gerar aleatório")) {
            static std::mt19937 gen{std::random_device{}()};
            static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
            m_q3.vecA = { dis(gen), dis(gen) };
            m_q3.vecB = { dis(gen), dis(gen) };
        }

        switch (m_q3.operation) {
            case 0: m_q3.resultVec = m_q3.vecA + m_q3.vecB; break;
            case 1: m_q3.resultVec = m_q3.vecA - m_q3.vecB; break;
            case 2: m_q3.resultScalar = cross(m_q3.vecA, m_q3.vecB); break;
            case 3: m_q3.resultScalar = m_q3.vecA.dot(m_q3.vecB); break;
        }
    }

    // -----------------------------------
    // Canvas
    // -----------------------------------
    void drawCanvas(const ImVec2& avail) {
        ImGui::SameLine();

        float canvasWidth = avail.x - m_panelWidth - m_splitterW;

        ImGui::BeginChild(
            "Canvas",
            {canvasWidth, avail.y},
            true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos   = ImGui::GetCursorScreenPos();
        ImVec2 size  = ImGui::GetContentRegionAvail();
        ImVec2 center = pos + size * 0.5f;

        ImGui::InvisibleButton("canvas_area", size);
        draw->AddRectFilled(pos, pos + size, IM_COL32(30, 30, 30, 255));

        float scale = std::min(size.x, size.y) * 0.4f;

        switch (m_questao) {
            case QUESTAO_1:
                drawVector(draw, center, m_q1.vecA, scale, IM_COL32(0, 140, 255, 255));
                drawVector(draw, center, m_q1.vecB, scale, IM_COL32(255, 255, 255, 255));
                break;

            case QUESTAO_2:
                drawVector(draw, center, m_q2.vec, scale, IM_COL32(0, 200, 255, 255));
                break;

            case QUESTAO_3:
                drawVector(draw, center, m_q3.vecA, scale, IM_COL32(0, 140, 255, 255));
                drawVector(draw, center, m_q3.vecB, scale, IM_COL32(255, 255, 255, 255));

                if (m_q3.operation < 2)
                    drawVector(draw, center, m_q3.resultVec, scale, IM_COL32(255, 220, 0, 255));
                else
                    draw->AddCircle(
                        center,
                        std::fabs(m_q3.resultScalar) * scale * 0.1f,
                        m_q3.resultScalar >= 0 ? IM_COL32(0, 220, 0, 255) : IM_COL32(220, 0, 0, 255), 
                        32, 2.0f
                    );
                break;
        }

        ImGui::EndChild();
    }

    // -----------------------------------
    // Utilitário gráfico
    // -----------------------------------
    void drawVector(ImDrawList* draw, const ImVec2& center,
                    const Vec2& v, float scale, ImU32 color) {
        ImVec2 end {
            center.x + v[0] * scale,
            center.y - v[1] * scale
        };
        draw->AddLine(center, end, color, 2.0f);
    }
};

int main() {
    Trabalho02 app;
    app.run(800, 600, "Trabalho 02 - Geometria Computacional");
}