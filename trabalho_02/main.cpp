#include "renderer.hpp"
#include "input.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>

class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

protected:
    void onUpdate(float dt) override {
        if (input().keys[GLFW_KEY_W]) {
            // mover câmera para frente
        }

        if (input().mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
            // interação
        }
    }

    void onUI() override {
        ImGui::Begin("Debug");
        ImGui::Text("Mouse: %.1f %.1f",
            input().mouseX,
            input().mouseY);
        ImGui::End();
    }
};

int main() {
    Trabalho02 app(800, 600, "Trabalho 02");
    app.run();
}