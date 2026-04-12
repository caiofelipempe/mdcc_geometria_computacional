#include "renderer.hpp"
#include "input.h"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

enum Questoes {
    QUESTAO_1 = 1,
    QUESTAO_2,
    QUESTAO_3,
    QUESTAO_4,
    QUESTAO_5,
    QUESTAO_6
};

class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

protected:
    Questoes m_questao = QUESTAO_1;

    void onInit() override {
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
            drawHorizontalLine();
            drawVerticalLine();
            break;
        case Questoes::QUESTAO_2:
            // TODO: Implementar update da questão 2.
            break;
        case Questoes::QUESTAO_3:
            // TODO: Implementar update da questão 3.
            break;
        case Questoes::QUESTAO_4:
            // TODO: Implementar update da questão 4.
            break;
        case Questoes::QUESTAO_5:
            // TODO: Implementar update da questão 5.
            break;
        case Questoes::QUESTAO_6:
            // TODO: Implementar update da questão 6.
            break;
        default:
            break;
        }
    }

    void onUI() override {
        ImGui::Begin("Debug");
        ImGui::Text("Mouse: %.1f %.1f",
            input().mouseX,
            input().mouseY);
        ImGui::End();

        ImGui::Begin("Questões");
        ImGui::Text("Questão %d", m_questao);
        if(ImGui::Button("Questão 1")) {
            m_questao = QUESTAO_1;
        }
        if(ImGui::Button("Questão 2")) {
            m_questao = QUESTAO_2;
        }
        if(ImGui::Button("Questão 3")) {
            m_questao = QUESTAO_3;
        }
        if(ImGui::Button("Questão 4")) {
            m_questao = QUESTAO_4;
        }
        if(ImGui::Button("Questão 5")) {
            m_questao = QUESTAO_5;
        }
        if(ImGui::Button("Questão 6")) {
            m_questao = QUESTAO_6;
        }
        ImGui::End();

        switch (m_questao)
        {
        case Questoes::QUESTAO_1:
            // TODO: Implementar UI da questão 1.
            break;
        case Questoes::QUESTAO_2:
            // TODO: Implementar UI da questão 2.
            break;
        case Questoes::QUESTAO_3:
            // TODO: Implementar UI da questão 3.
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
    }

    void onShutdown() override {
    }

private:
    void drawHorizontalLine() {
        glColor3f(0.0f, 1.0f, 0.0f);  // Verde
        glBegin(GL_LINES);
        glVertex2f(-1.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glEnd();
    }

    void drawVerticalLine() {
        glColor3f(1.0f, 1.0f, 0.0f);  // Amarelo
        glBegin(GL_LINES);
        glVertex2f(0.0f, -1.0f);
        glVertex2f(0.0f, 1.0f);
        glEnd();
    }

};

int main() {
    Trabalho02 app(800, 600, "Trabalho 02");
    app.run();
}