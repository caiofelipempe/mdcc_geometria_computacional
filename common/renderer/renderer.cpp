#include "renderer.hpp"
#include "input.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>
#include <chrono>


Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::initGLFW(const int w, const int h, const std::string& t) {
    if (!glfwInit())
        throw std::runtime_error("Erro ao iniciar GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    m_window = glfwCreateWindow(
        w, h, t.c_str(), nullptr, nullptr);

    if (!m_window)
        throw std::runtime_error("Erro ao criar janela");

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    
    glViewport(0, 0, w, h);

    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);
    glfwSetWindowUserPointer(m_window, this);
}

void Renderer::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Renderer::shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::run(const int w, const int h, const std::string& t) {
    initGLFW(w, h, t);
    initImGui();
    onInit(w, h, t);

    using clock = std::chrono::steady_clock;

    const double targetFPS  = 60.0;
    const double frameTime = 1.0 / targetFPS;

    auto last = clock::now();
    auto nextFrame = last;

    while (!glfwWindowShouldClose(m_window)) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        glfwPollEvents();
        m_input.resetFrameData();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        onUpdate(dt);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);

        nextFrame += std::chrono::duration<double>(frameTime);
        std::this_thread::sleep_until(nextFrame);
    }

    onShutdown();
    shutdownImGui();

    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

const InputState& Renderer::input() const {
    return m_input;
}

// Implementação das funções virtuais (com implementações vazias)
void Renderer::onInit(const int initialWidth, const int initialHeight, const std::string& initialTitle) {
}

void Renderer::onUpdate(float dt) {
}

void Renderer::onUI() {
}

void Renderer::onShutdown() {
}

void Renderer::onWindowResize(int width, int height) {
}

// Callbacks estáticos
void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Renderer* instance = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!instance) return;

    if (key >= 0 && key < 512) {
        instance->m_input.keys[key] = action != GLFW_RELEASE;
    }
}

void Renderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Renderer* instance = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!instance) return;

    if (button >= 0 && button < 8) {
        instance->m_input.mouseButtons[button] = action == GLFW_PRESS;
    }
}

void Renderer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Renderer* instance = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!instance) return;

    instance->m_input.mouseX = xpos;
    instance->m_input.mouseY = ypos;
}

void Renderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Renderer* instance = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!instance) return;

    instance->m_input.scrollOffset = yoffset;
}

void Renderer::windowSizeCallback(GLFWwindow* window, int width, int height) {
    Renderer* instance = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!instance) return;

    instance->onWindowResize(width, height);
}