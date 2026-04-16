#include "renderer.hpp"
#include "input.h"
#include "vecn.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <numbers>
#include <random>

typedef geometry::VecN<float, 2> Vec2;

// ─────────────────────────────────────────────
//  Constantes globais
// ─────────────────────────────────────────────
static constexpr int   PANEL_WIDTH = 200;
static constexpr float PI          = std::numbers::pi_v<float>;

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
    int  selectedOp = SOMA;
    Vec2 vec0{};
    Vec2 vec1{};
    Vec2 vecr{};
    float prodr = 0.0f;
};

struct Q4State {
    // TODO: definir quando a especificação estiver disponível
};

struct Q5State {
    // TODO: definir quando a especificação estiver disponível
};

struct Q6State {
    // TODO: definir quando a especificação estiver disponível
};

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

private:
    // ── janela ──────────────────────────────
    int m_width  = 800;
    int m_height = 600;

    // ── seleção de questão ──────────────────
    Questao m_questao            = Questao::Q1;
    int     m_questaoComboIndex  = 0;

    static inline constexpr const char* QUESTAO_ITEMS[] = {
        "Questão 1", "Questão 2", "Questão 3",
        "Questão 4", "Questão 5", "Questão 6"
    };
    static inline constexpr const char* Q3_OP_ITEMS[] = {
        "Soma", "Subtração", "Produto Vetorial", "Produto Escalar"
    };

    // ── estados por questão ─────────────────
    Q1State m_q1;
    Q2State m_q2;
    Q3State m_q3;
    Q4State m_q4;
    Q5State m_q5;
    Q6State m_q6;

    // ── RNG reutilizável ────────────────────
    std::mt19937                          m_rng{ std::random_device{}() };
    std::uniform_real_distribution<float> m_dist{ -1.0f, 1.0f };

    // ─────────────────────────────────────────
    //  Callbacks do Renderer
    // ─────────────────────────────────────────
protected:
    void onInit(const int initialWidth, const int initialHeight, const std::string& /*title*/) override {
            onWindowResize(initialWidth, initialHeight);
    }

    void onWindowResize(int width, int height) override {
        m_width  = width;
        m_height = height;
        glViewport(PANEL_WIDTH, 0, width - PANEL_WIDTH, height);
    }

    // onUpdate: apenas lógica de estado — sem cálculos dentro do onUI
    void onUpdate(float /*dt*/) override {
        auto canvasOrigin = Vec2({PANEL_WIDTH, 0});
        auto canvasCenter = Vec2({static_cast<float>(PANEL_WIDTH + (m_width - PANEL_WIDTH) / 2), static_cast<float>(m_height) / 2.0f});

        switch (m_questao) {
        case Questao::Q1:
            renderQ1();
            break;
        case Questao::Q2:
            renderQ2();
            break;
        case Questao::Q3:
            updateQ3();
            renderQ3();
            break;
        case Questao::Q4:
            // TODO: renderQ4()
            break;
        case Questao::Q5:
            // TODO: renderQ5()
            break;
        case Questao::Q6:
            // TODO: renderQ6()
            break;
        default:
            break;
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
        ImGui::Begin("##main_panel", nullptr,
            ImGuiWindowFlags_NoMove      |
            ImGuiWindowFlags_NoResize    |
            ImGuiWindowFlags_NoTitleBar  |
            ImGuiWindowFlags_NoSavedSettings);

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
        case Questao::Q4: uiQ4(); break;
        case Questao::Q5: uiQ5(); break;
        case Questao::Q6: uiQ6(); break;
        default: break;
        }

        ImGui::End();
    }

    void onShutdown() override {}

    // ─────────────────────────────────────────
    //  Lógica de update por questão
    // ─────────────────────────────────────────
private:
    void updateQ3() {
        switch (m_q3.selectedOp) {
        case Q3State::SOMA:      m_q3.vecr  = m_q3.vec0 + m_q3.vec1;           break;
        case Q3State::SUBTRACAO: m_q3.vecr  = m_q3.vec0 - m_q3.vec1;           break;
        case Q3State::CROSS:     m_q3.prodr = cross(m_q3.vec0, m_q3.vec1);     break;
        case Q3State::DOT:       m_q3.prodr = m_q3.vec0.dot(m_q3.vec1);        break;
        default: break;
        }
    }

    // ─────────────────────────────────────────
    //  Render OpenGL por questão
    // ─────────────────────────────────────────
    void renderQ1() {
        drawAxes();
        drawVectorLine(m_q1.vec0, 0.0f, 1.0f, 0.0f);
        drawVectorLine(m_q1.vec1, 0.0f, 0.0f, 1.0f);
    }

    void renderQ2() {
        drawAxes();
        drawVectorLine(m_q2.vec, 0.0f, 1.0f, 0.0f);
    }

    void renderQ3() {
        drawAxes();
        drawVectorLine(m_q3.vec0 / 2, 0.0f, 1.0f, 0.0f);
        drawVectorLine(m_q3.vec1 / 2, 0.0f, 0.0f, 1.0f);

        const bool vectorResult = (m_q3.selectedOp == Q3State::SOMA ||
                                   m_q3.selectedOp == Q3State::SUBTRACAO);
        if (vectorResult) {
            drawVectorLine(m_q3.vecr / 2, 1.0f, 1.0f, 0.0f);
        } else {
            drawCircle(0.0f, 0.0f, m_q3.prodr / 2, 40, 1.0f, 1.0f, 0.0f);
        }
    }

    // ─────────────────────────────────────────
    //  UI ImGui por questão
    // ─────────────────────────────────────────
void uiQ1() {
    const auto pa0 = m_q1.vec0.pseudoangle();
    const auto pa1 = m_q1.vec1.pseudoangle();
    const auto paDiff = geometry::pseudoangleBetween(m_q1.vec0, m_q1.vec1);

    ImGui::Text("Vetor 1:");
    ImGui::DragFloat2("##q1_v0", &m_q1.vec0[0], 0.01f, -1.0f, 1.0f);
    warnIfZero(m_q1.vec0);
    if (pa0.has_value())
        ImGui::Text("Pseudoângulo: %.4f", pa0.value());
    else
        ImGui::TextDisabled("Pseudoângulo: N/A");

    ImGui::Separator();
    ImGui::Text("Vetor 2:");
    ImGui::DragFloat2("##q1_v1", &m_q1.vec1[0], 0.01f, -1.0f, 1.0f);
    warnIfZero(m_q1.vec1);
    if (pa1.has_value())
        ImGui::Text("Pseudoângulo: %.4f", pa1.value());
    else
        ImGui::TextDisabled("Pseudoângulo: N/A");

    ImGui::Separator();
    if (paDiff.has_value()) {
        const float diff = paDiff.value() >= 0.0f ? paDiff.value()
                                                  : 8.0f + paDiff.value();
        ImGui::Text("Δ pseudoângulo: %.4f", diff);
        if (m_q1.vec0.pseudoangle_less(m_q1.vec1))
            ImGui::Text("Vetor 1 está à direita de 2");
        else if (m_q1.vec1.pseudoangle_less(m_q1.vec0))
            ImGui::Text("Vetor 1 está à esquerda de 2");
        else
            ImGui::Text("Ângulos iguais");
    }
}

void uiQ2() {
    ImGui::Text("Vetor:");
    ImGui::DragFloat2("##q2_v", &m_q2.vec[0], 0.01f, -1.0f, 1.0f);
    warnIfZero(m_q2.vec);

    ImGui::Separator();
    const auto pa    = m_q2.vec.pseudoangle();
    const auto paAlt = m_q2.vec.pseudoangleAlt();
    if (pa.has_value()) {
        ImGui::Text("Pseudoângulo (octante): %.4f",  pa.value());
        ImGui::Text("Pseudoângulo (quadrante): %.4f", paAlt.value());
    } else {
        ImGui::TextColored({1.0f, 0.4f, 0.4f, 1.0f}, "Vetor inválido");
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Explicação")) {
        ImGui::TextWrapped(
            "O algoritmo de octante divide o plano em 8 regiões usando "
            "tangente/cotangente, mapeando o resultado em [0, 8). "
            "O de quadrante é mais simples (x/(|x|+|y|) ou similar), "
            "mapeando em [0, 4). Ambos produzem resultados consistentes "
            "para a maioria dos vetores, mas o de octante é numericamente "
            "mais estável próximo aos eixos."
        );
    }
}

void uiQ3() {
    ImGui::Text("Operação:");
    ImGui::Combo("##q3_op", &m_q3.selectedOp,
                 Q3_OP_ITEMS, IM_ARRAYSIZE(Q3_OP_ITEMS));
    ImGui::Separator();

    ImGui::Text("Vetor 1:");
    ImGui::DragFloat2("##q3_v0", &m_q3.vec0[0], 0.01f, -1.0f, 1.0f);
    warnIfZero(m_q3.vec0);

    ImGui::Separator();
    ImGui::Text("Vetor 2:");
    ImGui::DragFloat2("##q3_v1", &m_q3.vec1[0], 0.01f, -1.0f, 1.0f);
    warnIfZero(m_q3.vec1);

    ImGui::Spacing();
    if (ImGui::Button("Gerar aleatório")) {
        m_q3.vec0[0] = m_dist(m_rng);
        m_q3.vec0[1] = m_dist(m_rng);
        m_q3.vec1[0] = m_dist(m_rng);
        m_q3.vec1[1] = m_dist(m_rng);
    }
    ImGui::Spacing();
    ImGui::Separator();

    switch (m_q3.selectedOp) {
    case Q3State::SOMA:
        ImGui::Text("Soma: (%.4f, %.4f)", m_q3.vecr[0], m_q3.vecr[1]);
        break;
    case Q3State::SUBTRACAO:
        ImGui::Text("Subtração: (%.4f, %.4f)", m_q3.vecr[0], m_q3.vecr[1]);
        break;
    case Q3State::CROSS:
        ImGui::Text("Produto vetorial: %.4f", m_q3.prodr);
        break;
    case Q3State::DOT:
        ImGui::Text("Produto escalar: %.4f", m_q3.prodr);
        break;
    default:
        break;
    }
}

    void uiQ4() { ImGui::TextDisabled("(não implementado)"); }
    void uiQ5() { ImGui::TextDisabled("(não implementado)"); }
    void uiQ6() { ImGui::TextDisabled("(não implementado)"); }

    // ─────────────────────────────────────────
    //  Helpers de desenho
    // ─────────────────────────────────────────
    void drawAxes() {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(-1.0f,  0.0f);
        glVertex2f( 1.0f,  0.0f);
        glEnd();

        glBegin(GL_LINES);
        glVertex2f(0.0f, -1.0f);
        glVertex2f(0.0f,  1.0f);
        glEnd();
    }

    void drawVectorLine(const Vec2& v, float r = 1.0f, float g = 1.0f, float b = 0.0f) {
        // escalar para manter proporção
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        if (m_width > m_height)
            scaleX = static_cast<float>(m_height) / static_cast<float>(m_width);
        else
            scaleY = static_cast<float>(m_width)  / static_cast<float>(m_height);

        glColor3f(r, g, b);
        glBegin(GL_LINES);
        glVertex2f(0.0f,         0.0f);
        glVertex2f(v[0] * scaleX, v[1] * scaleY);
        glEnd();
    }

    void drawCircle(float cx, float cy, float radius, int segments,
                    float r = 0.0f, float g = 0.0f, float b = 1.0f) {
        glColor3f(r, g, b);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            const float theta = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
            glVertex2f(cx + radius * std::cos(theta),
                       cy + radius * std::sin(theta));
        }
        glEnd();
    }

    // ─────────────────────────────────────────
    //  Helper de UI
    // ─────────────────────────────────────────
    static void warnIfZero(const Vec2& v) {
        if (v[0] == 0.0f && v[1] == 0.0f)
            ImGui::TextColored({1.0f, 0.4f, 0.4f, 1.0f}, "⚠ Vetor nulo");
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho02 app;
    app.run(800, 600, "Trabalho 02 - Geometria Computacional");
}