#include "renderer.hpp"
#include "input.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>
#include <chrono>

static Renderer* s_instance = nullptr;

Renderer::Renderer(int w, int h, const std::string& t)
    : m_width(w), m_height(h), m_title(t) {

    s_instance = this;
    m_input = new InputState();

    initGLFW();
    initImGui();
    onInit();
}

Renderer::~Renderer() {
    onShutdown();
    shutdownImGui();

    glfwDestroyWindow(m_window);
    glfwTerminate();

    delete m_input;
}

void Renderer::initGLFW() {
    if (!glfwInit())
        throw std::runtime_error("Erro ao iniciar GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(
        m_width, m_height, m_title.c_str(), nullptr, nullptr);

    if (!m_window)
        throw std::runtime_error("Erro ao criar janela");

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
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

void Renderer::run() {
    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();

    while (!glfwWindowShouldClose(m_window)) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        glfwPollEvents();
        m_input->resetFrameData();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onUpdate(dt);
        onUI();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

const InputState& Renderer::input() const {
    return *m_input;
}