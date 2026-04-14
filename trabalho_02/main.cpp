#include "renderer.hpp"
#include "input.h"
#include "vecn.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <random>

typedef geometry::VecN<float, 2> Vec2;

enum Questoes {
    NO_SELECTION = 0,
    QUESTAO_1,
    QUESTAO_2,
    QUESTAO_3,
    QUESTAO_4,
    QUESTAO_5,
    QUESTAO_6
};

class Trabalho02 : public Renderer {
private:
    int m_width;
    int m_height;
    static constexpr const char* questaoItems[] = { "Questão 1", "Questão 2", "Questão 3", "Questão 4", "Questão 5", "Questão 6" };
    int questaoSelectedItem = m_questao;

    // Variaveis da Questão 1
    Vec2 m_q1_vec0;
    Vec2 m_q1_vec1;

    // Variaveis da Questão 2
    Vec2 m_q2_vec;

    // Variaveis da Questão 3
    static constexpr const char* m_q3_operacaoItems[] = { "Soma", "Subtração", "Produto Vetorial", "Produto Scalar" };
    int m_q3_selectedOperacaoItem = 0;
    Vec2 m_q3_vec0;
    Vec2 m_q3_vec1;
    Vec2 m_q3_vecr;
    float m_q3_prodr;

public:
    using Renderer::Renderer;

protected:
    Questoes m_questao = QUESTAO_1;

    void onInit(const int initialWidth, const int initialHeight, const std::string& initialTitle) override {
        m_height = initialHeight;
        m_width = initialWidth;
    }

    void onWindowResize(int width, int height) override {
        m_height = height;
        m_width = width;

        glViewport(0, 0, width, height);
    }

    void onUpdate(float dt) override {
        if (input().keys[GLFW_KEY_W]) {
            // mover câmera para frente
        }

        if (input().mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
            // interação
        }

        switch (m_questao)
        {
        case Questoes::QUESTAO_1:
            {
                drawHorizontalLine();
                drawVerticalLine();
                drawVectorLine(m_q1_vec0, 0.0, 1.0, 0.0);
                drawVectorLine(m_q1_vec1, 0.0, 0.0, 1.0);
            }
            break;
        case Questoes::QUESTAO_2:
            {
                drawHorizontalLine();
                drawVerticalLine();
                drawVectorLine(m_q2_vec, 0.0, 1.0, 0.0);
            }
            break;
        case Questoes::QUESTAO_3:
            {
                float l1 = m_q3_vec0.length_squared();
                float l2 = m_q3_vec1.length_squared();
                float lr = m_q3_selectedOperacaoItem < 2 ? m_q3_vecr.length_squared() : m_q3_prodr*m_q3_prodr;
                float f = 1;
                if(l1 > 1 || l2 > 1 || lr > 1) {
                    f = std::max(lr, std::max(l1, l2));
                }

                drawHorizontalLine();
                drawVerticalLine();
                drawVectorLine(m_q3_vec0/f, 0.0, 1.0, 0.0);
                drawVectorLine(m_q3_vec1/f, 0.0, 0.0, 1.0);
                if (m_q3_selectedOperacaoItem == 0 || m_q3_selectedOperacaoItem == 1) {
                    drawVectorLine(m_q3_vecr/f, 1.0, 1.0, 0.0);
                } else if (m_q3_selectedOperacaoItem == 2 || m_q3_selectedOperacaoItem == 3) {
                    DrawCircle(0.0, 0.0, m_q3_prodr/f, 20, 1.0, 1.0, 0.0);
                }
            }
            break;
        case Questoes::QUESTAO_4:
            {
                // TODO: Implementar update da questão 4.
            }
            break;
        case Questoes::QUESTAO_5:
            {
                // TODO: Implementar update da questão 5.
            }
            break;
        case Questoes::QUESTAO_6:
            {
                // TODO: Implementar update da questão 6.
            }
            break;
        default:
            break;
        }
    }

    void onUI() override {
#ifndef NDEBUG
        ImGui::Begin("Debug");
        ImGui::Text("Mouse: %.1f %.1f",
            input().mouseX,
            input().mouseY);
        ImGui::End();
#endif

        ImGui::Begin(std::string("Questão ").append(std::to_string(m_questao)).c_str());
        ImGui::Text("Selecione a questão:");
        if (ImGui::Combo("", &questaoSelectedItem, questaoItems, IM_ARRAYSIZE(questaoItems))) {
            m_questao = static_cast<Questoes>(questaoSelectedItem + 1);
        }
        ImGui::Separator();
        switch (m_questao)
        {
        case Questoes::QUESTAO_1:
            {
                auto pseudoangle0 = m_q1_vec0.pseudoangle();
                auto pseudoangle1 = m_q1_vec1.pseudoangle();
                auto pseudoangle_diff = geometry::pseudoangleBetween(m_q1_vec0, m_q1_vec1);
                ImGui::Text("Vetor 1:");
                ImGui::SliderFloat2("##m_q1_vec0", &m_q1_vec0[0], -1.0f, 1.0f);
                pseudoangle0.has_value() ? ImGui::Text("Pseudoangulo de vetor1: %f", pseudoangle0.value_or(-1.0f)) : ImGui::Text("Pseudoangulo de vetor1: N/A");
                ImGui::Separator();
                ImGui::Text("Vetor 2:");
                ImGui::SliderFloat2("##m_q1_vec1", &m_q1_vec1[0], -1.0f, 1.0f);
                pseudoangle1.has_value() ? ImGui::Text("Pseudoangulo de vetor2: %f", pseudoangle1.value_or(-1.0f)) : ImGui::Text("Pseudoangulo de vetor2: N/A");
                ImGui::Separator();
                if (pseudoangle_diff.has_value()) {
                    ImGui::Text("Diferença entre pseudoângulos: %f", pseudoangle_diff.value() >= 0.0 ? pseudoangle_diff.value() : 8.0 + pseudoangle_diff.value());
                    if (m_q1_vec0.pseudoangle_less(m_q1_vec1))
                        ImGui::Text("Vetor 1 está a direita de 2");
                    else if (m_q1_vec1.pseudoangle_less(m_q1_vec0))
                        ImGui::Text("Vetor 1 está a esquerda de 2");
                    else
                        ImGui::Text("Vetor 1 e vetor 2 têm ângulos iguais");
                }
            }
            break;
        case Questoes::QUESTAO_2:
            {
                auto pseudoangle0 = m_q2_vec.pseudoangle();
                auto pseudoangle1 = m_q2_vec.pseudoangleAlt();
                ImGui::Text("Vetor:");
                ImGui::SliderFloat2("##m_q2_vec", &m_q2_vec[0], -1.0f, 1.0f);
                pseudoangle0.has_value() ? ImGui::Text("Pseudoangulo: %f\nPseudoangulo alternativo: %f", pseudoangle0.value_or(-1.0f), pseudoangle1.value_or(-1.0f)) : ImGui::Text("Vetor invalido para cálculo de pseudoângulo");
                ImGui::Separator();
                if (ImGui::CollapsingHeader("Explicação")) {
                    ImGui::Text("O pseudoângulo alternativo deve ser igual à metade do pseudoângulo original,\nou seja, deve mapear o plano em uma faixa\nde [0, 4) ao invés de [0, 8).\nO segundo algoritmo é mais simples,\nmas o primeiro pode ser mais preciso em alguns casos,\nespecialmente para vetores próximos aos eixos,\nonde a divisão por valores pequenos pode causar instabilidade numérica.\nNo entanto, ambos os algoritmos devem produzir resultados\nconsistentes para a maioria dos vetores,\ne a escolha entre eles pode depender do contexto específico de uso.\nO algoritmo alternativo calcula o pseudoângulo pelo\nvalor do quadrante + ou - x/(x*y) ou y/(x*y),\nenquanto o algoritmo original calcula o pseudoângulo\nusando uma abordagem que divide o plano em 8 octantes,\nusando tangente e cotangente para o cálculo.");
                }
            }
            break;
        case Questoes::QUESTAO_3:
            {
                ImGui::Text("Selecione a operação:");
                ImGui::Combo("##m_q3_selectedOperacaoItem", &m_q3_selectedOperacaoItem, m_q3_operacaoItems, IM_ARRAYSIZE(m_q3_operacaoItems));
                ImGui::Text("Vetor 1:");
                ImGui::SliderFloat2("##m_q3_vec0", &m_q3_vec0[0], -1.0f, 1.0f);
                ImGui::Separator();
                ImGui::Text("Vetor 2:");
                ImGui::SliderFloat2("##m_q3_vec1", &m_q3_vec1[0], -1.0f, 1.0f);
                ImGui::Separator();
                if (ImGui::Button("Gerar aleatório")) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, 10000);
                    m_q3_vec0[0] = dis(gen)/10000.0;
                    m_q3_vec0[1] = dis(gen)/10000.0;
                    m_q3_vec1[0] = dis(gen)/10000.0;
                    m_q3_vec1[1] = dis(gen)/10000.0;
                }
                ImGui::Separator();
                switch (m_q3_selectedOperacaoItem) {
                    case 0:
                        {
                            m_q3_vecr = m_q3_vec0 + m_q3_vec1;
                            ImGui::Text("Resultado da soma: (%f, %f)", m_q3_vecr[0], m_q3_vecr[1]);
                        }
                        break;
                    case 1:
                        {
                            m_q3_vecr = m_q3_vec0 - m_q3_vec1;
                            ImGui::Text("Resultado da subtração: (%f, %f)", m_q3_vecr[0], m_q3_vecr[1]);
                        }
                        break;
                    case 2:
                        {
                            m_q3_prodr = cross(m_q3_vec0, m_q3_vec1);
                            ImGui::Text("Resultado do produto vetorial: %f", m_q3_prodr);
                        }
                        break;
                    case 3:
                        {
                            m_q3_prodr = m_q3_vec0.dot(m_q3_vec1);
                            ImGui::Text("Resultado do produto escalar: %f", m_q3_prodr);
                        }
                        break;
                }
            }
            break;
        case Questoes::QUESTAO_4:
            // TODO: Implementar UI da questão 4.
            break;
        case Questoes::QUESTAO_5:
            // TODO: Implementar UI da questão 5.
            break;
        case Questoes::QUESTAO_6:
            // TODO: Implementar UI da questão 6.
            break;
        default:
            break;
        }
        ImGui::End();
    }

    void onShutdown() override {
    }

private:
    void drawHorizontalLine() {
        glColor3f(1.0f, 1.0f, 1.0f);  // Verde
        glBegin(GL_LINES);
        glVertex2f(-1.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glEnd();
    }

    void drawVerticalLine() {
        glColor3f(1.0f, 1.0f, 1.0f);  // Amarelo
        glBegin(GL_LINES);
        glVertex2f(0.0f, -1.0f);
        glVertex2f(0.0f, 1.0f);
        glEnd();
    }

    void drawVectorLine(const Vec2& v, float r = 1.0, float g = 1.0, float b = 0) {
        float h = 1.0f;
        float w = 1.0f;
        if(m_width > m_height) {
            w = static_cast<float>(m_height) / static_cast<float>(m_width);
        } else {
            h = static_cast<float>(m_width) / static_cast<float>(m_height);
        }

        glColor3f(r, g, b);  // Vermelho
        glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(v[0] * w, v[1] * h);
        glEnd();
    }

    void DrawCircle(float cx, float cy, float radius, int num_segments, float r = 0.0, float g = 0.0, float b = 1.0) {
        glBegin(GL_LINE_LOOP);
        glColor3f(r, g, b);
        for (int ii = 0; ii < num_segments; ii++)   {
            float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
            float x = radius * cosf(theta);//calculate the x component 
            float y = radius * sinf(theta);//calculate the y component 
            glVertex2f(x + cx, y + cy);//output vertex 
        }
        glEnd();
    }

};

int main() {
    Trabalho02 app;
    app.run(800, 600, "Trabalho 02 - Geometria Computacional");
}