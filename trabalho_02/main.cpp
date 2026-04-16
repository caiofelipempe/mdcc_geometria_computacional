#include "renderer.hpp"
#include "input.h"
#include "geometry_array.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <numbers>
#include <random>

using Vec2 = geometry::Vec<float, 2>;

// ─────────────────────────────────────────────
//  Constantes globais
// ─────────────────────────────────────────────
static constexpr int   PANEL_WIDTH = 200;
static constexpr float PI = std::numbers::pi_v<float>;

// ─────────────────────────────────────────────
//  Enum de questões
// ─────────────────────────────────────────────
enum class Questao {
    Q1 = 0,
    Q2,
    Q3,
    Q4,
    Q5,
    Q6,
    COUNT
};

// ─────────────────────────────────────────────
//  Estado por questão
// ─────────────────────────────────────────────
struct Q1State {
    Vec2 vec0{};
    Vec2 vec1{};
};

struct Q2State {
    Vec2 vec{};
};

struct Q3State {
    enum Op { SOMA = 0, SUBTRACAO, CROSS, DOT, COUNT };
    int   selectedOp = SOMA;
    Vec2  vec0{};
    Vec2  vec1{};
    Vec2  vecr{};
    float prodr = 0.0f;
};

struct Q4State {};
struct Q5State {};
struct Q6State {};

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

private:
    int m_width  = 800;
    int m_height = 600;

    Questao m_questao = Questao::Q1;
    int m_questaoComboIndex = 0;

    static inline constexpr const char* QUESTAO_ITEMS[] = {
        "Questão 1", "Questão 2", "Questão 3",
        "Questão 4", "Questão 5", "Questão 6"
    };

    static inline constexpr const char* Q3_OP_ITEMS[] = {
        "Soma", "Subtração", "Produto Vetorial", "Produto Escalar"
    };

    Q1State m_q1;
    Q2State m_q2;
    Q3State m_q3;
    Q4State m_q4;
    Q5State m_q5;
    Q6State m_q6;

    std::mt19937 m_rng{ std::random_device{}() };
    std::uniform_real_distribution<float> m_dist{ -1.0f, 1.0f };

protected:
    void onInit(int w, int h, const std::string&) override {
        onWindowResize(w, h);
    }

    void onWindowResize(int w, int h) override {
        m_width = w;
        m_height = h;
        glViewport(PANEL_WIDTH, 0, w - PANEL_WIDTH, h);
    }

    void onUpdate(float) override {
        switch (m_questao) {
        case Questao::Q1: renderQ1(); break;
        case Questao::Q2: renderQ2(); break;
        case Questao::Q3: {
                updateQ3();
                renderQ3();
            } break;
        default: break;
        }
    }

    void onUI() override {
#ifndef NDEBUG
        ImGui::Begin("Debug");
        ImGui::Text("Mouse: %.1f %.1f", input().mouseX, input().mouseY);
        ImGui::End();
#endif

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, m_height));
        ImGui::Begin("##panel", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("Selecione a questão:");
        if (ImGui::Combo("##questao", &m_questaoComboIndex,
                         QUESTAO_ITEMS, IM_ARRAYSIZE(QUESTAO_ITEMS))) {
            m_questao = static_cast<Questao>(m_questaoComboIndex);
        }

        ImGui::Separator();

        switch (m_questao) {
        case Questao::Q1: uiQ1(); break;
        case Questao::Q2: uiQ2(); break;
        case Questao::Q3: uiQ3(); break;
        default:{
            ImGui::TextDisabled("(não implementado)");
        } break;
        }

        ImGui::End();
    }

private:
    // ──────────────── Lógica ────────────────
    void updateQ3() {
        switch (m_q3.selectedOp) {
        case Q3State::SOMA:
            m_q3.vecr = m_q3.vec0 + m_q3.vec1;
            break;
        case Q3State::SUBTRACAO:
            m_q3.vecr = m_q3.vec0 - m_q3.vec1;
            break;
        case Q3State::CROSS:
            m_q3.prodr = geometry::cross(m_q3.vec0, m_q3.vec1);
            break;
        case Q3State::DOT:
            m_q3.prodr = geometry::dot(m_q3.vec0, m_q3.vec1);
            break;
        default: break;
        }
    }

    // ──────────────── Render ────────────────
    void drawAxes() {
        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(-1, 0); glVertex2f(1, 0);
        glVertex2f(0, -1); glVertex2f(0, 1);
        glEnd();
    }

    void drawVectorLine(const Vec2& v, float r, float g, float b) {
        glColor3f(r, g, b);
        glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(v[0], v[1]);
        glEnd();
    }

    void drawCircle(float r) {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 40; ++i) {
            float t = 2.0f * PI * i / 40.0f;
            glVertex2f(std::cos(t) * r, std::sin(t) * r);
        }
        glEnd();
    }

    void renderQ1() {
        drawAxes();
        drawVectorLine(m_q1.vec0, 0, 1, 0);
        drawVectorLine(m_q1.vec1, 0, 0, 1);
    }

    void renderQ2() {
        drawAxes();
        drawVectorLine(m_q2.vec, 0, 1, 0);
    }

    void renderQ3() {
        drawAxes();
        drawVectorLine(m_q3.vec0 * 0.5f, 0, 1, 0);
        drawVectorLine(m_q3.vec1 * 0.5f, 0, 0, 1);

        if (m_q3.selectedOp <= Q3State::SUBTRACAO)
            drawVectorLine(m_q3.vecr * 0.5f, 1, 1, 0);
        else
            drawCircle(m_q3.prodr * 0.5f);
    }

    // ──────────────── UI ────────────────
    void uiQ1() {
        auto pa0 = geometry::pseudoangle(m_q1.vec0);
        auto pa1 = geometry::pseudoangle(m_q1.vec1);

        ImGui::Text("Vetor 1:");
        ImGui::DragFloat2("##v0", &m_q1.vec0[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec0);
        if (pa0) ImGui::Text("Pseudoângulo: %.4f", pa0.value());

        ImGui::Separator();
        ImGui::Text("Vetor 2:");
        ImGui::DragFloat2("##v1", &m_q1.vec1[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec1);
        if (pa1) ImGui::Text("Pseudoângulo: %.4f", pa1.value());
    }

    void uiQ2() {
        ImGui::Text("Vetor:");
        ImGui::DragFloat2("##v", &m_q2.vec[0], 0.01f, -1, 1);
        warnIfZero(m_q2.vec);

        auto pa = geometry::pseudoangle(m_q2.vec);
        auto pb = geometry::pseudoangleAlt(m_q2.vec);

        if (pa) {
            ImGui::Text("Octante: %.4f", pa.value());
            ImGui::Text("Quadrante: %.4f", pb.value());
        }
    }

    void uiQ3() {
        ImGui::Combo("Operação", &m_q3.selectedOp,
                     Q3_OP_ITEMS, IM_ARRAYSIZE(Q3_OP_ITEMS));
        ImGui::Separator();

        ImGui::DragFloat2("Vetor A", &m_q3.vec0[0], 0.01f, -1, 1);
        ImGui::DragFloat2("Vetor B", &m_q3.vec1[0], 0.01f, -1, 1);

        if (ImGui::Button("Aleatório")) {
            m_q3.vec0 = { m_dist(m_rng), m_dist(m_rng) };
            m_q3.vec1 = { m_dist(m_rng), m_dist(m_rng) };
        }

        ImGui::Separator();
        if (m_q3.selectedOp == Q3State::CROSS || m_q3.selectedOp == Q3State::DOT)
            ImGui::Text("Resultado: %.4f", m_q3.prodr);
        else
            ImGui::Text("Resultado: (%.4f, %.4f)", m_q3.vecr[0], m_q3.vecr[1]);
    }

    static void warnIfZero(const Vec2& v) {
        if (v[0] == 0.0f && v[1] == 0.0f)
            ImGui::TextColored({1, 0.4f, 0.4f, 1}, "⚠ Vetor nulo");
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho02 app;
    app.run(800, 600, "Trabalho 02 - Geometria Computacional");
}
