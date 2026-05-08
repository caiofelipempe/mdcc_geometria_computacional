#include <GL/glew.h> 

#include "renderer.hpp"
#include "input.h"

#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

// ─────────────────────────────────────────────────────────────────────────────
//  Ctor / Dtor
// ─────────────────────────────────────────────────────────────────────────────
Renderer::Renderer()  = default;
Renderer::~Renderer() = default;

// ─────────────────────────────────────────────────────────────────────────────
//  GLFW
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::initGLFW(const int w, const int h, const std::string& t) {
    if (!glfwInit())
        throw std::runtime_error("Erro ao iniciar GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    m_window = glfwCreateWindow(w, h, t.c_str(), nullptr, nullptr);
    if (!m_window)
        throw std::runtime_error("Erro ao criar janela");

    glfwMakeContextCurrent(m_window);

    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Erro ao iniciar GLEW");
    }

    glfwSwapInterval(1);
    glViewport(0, 0, w, h);

    glfwSetWindowUserPointer   (m_window, this);
    glfwSetKeyCallback         (m_window, keyCallback);
    glfwSetMouseButtonCallback (m_window, mouseButtonCallback);
    glfwSetCursorPosCallback   (m_window, cursorPosCallback);
    glfwSetScrollCallback      (m_window, scrollCallback);
    glfwSetWindowSizeCallback  (m_window, windowSizeCallback);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
//  Loop principal
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::run(const int w, const int h, const std::string& t) {
    initGLFW(w, h, t);

    initImGui();
    onInit(w, h, t);

    using clock = std::chrono::steady_clock;
    constexpr double TARGET_FPS   = 60.0;
    constexpr double FRAME_TIME   = 1.0 / TARGET_FPS;

    auto last       = clock::now();
    auto nextFrame  = last;

    while (!glfwWindowShouldClose(m_window)) {
        const auto now = clock::now();
        const float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        glfwPollEvents();
        m_input.resetFrameData();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        onUpdate(dt);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        onUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);

        auto sleep_until = nextFrame + std::chrono::duration<double>(FRAME_TIME);
        std::this_thread::sleep_until(sleep_until);
    }

    onShutdown();
    shutdownImGui();

    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Accessors
// ─────────────────────────────────────────────────────────────────────────────
const InputState& Renderer::input() const { return m_input; }

// ─────────────────────────────────────────────────────────────────────────────
//  Virtuais com implementação vazia (base)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::onInit        (int, int, const std::string&) {}
void Renderer::onUpdate      (float) {}
void Renderer::onUI          () {}
void Renderer::onShutdown    () {}
void Renderer::onWindowResize(int width, int height) {
    (void)width; (void)height; // suprime warning no build OpenGL
}

// ─────────────────────────────────────────────────────────────────────────────
//  Callbacks GLFW (estáticos)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::keyCallback(GLFWwindow* window, int key, int /*scan*/, int action, int /*mods*/) {
    auto* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    if (key >= 0 && key < 512)
        self->m_input.keys[key] = (action != GLFW_RELEASE);
}

void Renderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    if (button >= 0 && button < 8)
        self->m_input.mouseButtons[button] = (action == GLFW_PRESS);
}

void Renderer::cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    self->m_input.mouseX = x;
    self->m_input.mouseY = y;
}

void Renderer::scrollCallback(GLFWwindow* window, double /*dx*/, double dy) {
    auto* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    self->m_input.scrollOffset = dy;
}

void Renderer::windowSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    self->onWindowResize(width, height);
}