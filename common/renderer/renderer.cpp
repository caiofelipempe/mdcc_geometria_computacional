#include "renderer.hpp"
#include "input.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#ifdef USE_BGFX
// ── bgfx ──────────────────────────────────────────────────────────────────
#   include <bgfx/bgfx.h>
#   include <bgfx/platform.h>
#   include <imgui_impl_bgfx.h>          // imgui-bgfx backend (bigg / bgfx-imgui)

// Obter handles nativos de janela dependendo da plataforma
#   if defined(_WIN32)
#       define GLFW_EXPOSE_NATIVE_WIN32
#   elif defined(__APPLE__)
#       define GLFW_EXPOSE_NATIVE_COCOA
#   else
#       define GLFW_EXPOSE_NATIVE_X11
#   endif
#   include <GLFW/glfw3native.h>

#else
// ── OpenGL ────────────────────────────────────────────────────────────────
#   include <imgui_impl_opengl3.h>
#   include <GL/glu.h>
#endif

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

#ifdef USE_BGFX
    // bgfx gerencia o contexto gráfico — pedir janela sem contexto OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

    m_window = glfwCreateWindow(w, h, t.c_str(), nullptr, nullptr);
    if (!m_window)
        throw std::runtime_error("Erro ao criar janela");

#ifndef USE_BGFX
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glViewport(0, 0, w, h);
#endif

    glfwSetWindowUserPointer   (m_window, this);
    glfwSetKeyCallback         (m_window, keyCallback);
    glfwSetMouseButtonCallback (m_window, mouseButtonCallback);
    glfwSetCursorPosCallback   (m_window, cursorPosCallback);
    glfwSetScrollCallback      (m_window, scrollCallback);
    glfwSetWindowSizeCallback  (m_window, windowSizeCallback);
}

// ─────────────────────────────────────────────────────────────────────────────
//  bgfx — inicialização / shutdown / helpers de plataforma
// ─────────────────────────────────────────────────────────────────────────────
#ifdef USE_BGFX

void* Renderer::getNativeWindowHandle() const {
#if defined(_WIN32)
    return glfwGetWin32Window(m_window);
#elif defined(__APPLE__)
    return glfwGetCocoaWindow(m_window);
#else
    return reinterpret_cast<void*>(glfwGetX11Window(m_window));
#endif
}

void* Renderer::getNativeDisplayHandle() const {
#if defined(_WIN32) || defined(__APPLE__)
    return nullptr;
#else
    return glfwGetX11Display();
#endif
}

void Renderer::initBgfx(const int w, const int h) {
    m_bgfxWidth  = w;
    m_bgfxHeight = h;

    // Informar o bgfx sobre a janela nativa ANTES de Init()
    bgfx::PlatformData pd{};
    pd.nwh  = getNativeWindowHandle();
    pd.ndt  = getNativeDisplayHandle();
    bgfx::setPlatformData(pd);

    bgfx::Init init{};
    init.type              = bgfx::RendererType::Count; // auto-detectar backend
    init.vendorId          = BGFX_PCI_ID_NONE;
    init.resolution.width  = static_cast<uint32_t>(w);
    init.resolution.height = static_cast<uint32_t>(h);
    init.resolution.reset  = BGFX_RESET_VSYNC;

    if (!bgfx::init(init))
        throw std::runtime_error("Erro ao inicializar bgfx");

    bgfx::setViewClear(m_viewId,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x1a1a1aff,   // background ~cinza escuro (equivalente ao glClearColor anterior)
        1.0f, 0);

    bgfx::setViewRect(m_viewId, 0, 0,
        static_cast<uint16_t>(w),
        static_cast<uint16_t>(h));
}

void Renderer::shutdownBgfx() {
    bgfx::shutdown();
}

#endif // USE_BGFX

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

#ifdef USE_BGFX
    ImGui_ImplGlfw_InitForOther(m_window, true);
    ImGui_Implbgfx_Init(m_viewId);
#else
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
}

void Renderer::shutdownImGui() {
#ifdef USE_BGFX
    ImGui_Implbgfx_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
#endif
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Loop principal
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::run(const int w, const int h, const std::string& t) {
    initGLFW(w, h, t);

#ifdef USE_BGFX
    initBgfx(w, h);
#endif

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

#ifdef USE_BGFX
        // Sinalizar ao bgfx que este é o início de um frame para a view
        bgfx::touch(m_viewId);
        onUpdate(dt);

        ImGui_Implbgfx_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        onUI();
        ImGui::Render();
        ImGui_Implbgfx_RenderDrawData(ImGui::GetDrawData());

        bgfx::frame();
#else
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
#endif

        auto sleep_until = nextFrame + std::chrono::duration<double>(FRAME_TIME);
        std::this_thread::sleep_until(sleep_until);
    }

    onShutdown();
    shutdownImGui();

#ifdef USE_BGFX
    shutdownBgfx();
#endif

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
#ifdef USE_BGFX
    m_bgfxWidth  = width;
    m_bgfxHeight = height;
    bgfx::reset(
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        BGFX_RESET_VSYNC);
    bgfx::setViewRect(m_viewId, 0, 0,
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height));
#endif
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