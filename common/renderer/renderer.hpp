#pragma once

#include <string>
#include "input.h"

struct GLFWwindow;

#ifdef USE_BGFX
#   include <bgfx/bgfx.h>
#   include <bgfx/platform.h>
#endif

class Renderer {
public:
    Renderer();
    virtual ~Renderer();

    void run(int w, int h, const std::string& t);

protected:
    virtual void onInit(int initialWidth, int initialHeight, const std::string& initialTitle);
    virtual void onUpdate(float dt);
    virtual void onUI();
    virtual void onShutdown();
    virtual void onWindowResize(int width, int height);

    const InputState& input() const;

#ifdef USE_BGFX
    // Submissão de draw calls bgfx — view padrão 0
    // Subclasses podem usar viewId() para obter a view atual
    bgfx::ViewId viewId() const { return m_viewId; }
#endif

private:
    void initGLFW(int w, int h, const std::string& t);
    void initImGui();
    void shutdownImGui();

#ifdef USE_BGFX
    void initBgfx(int w, int h);
    void shutdownBgfx();
    void* getNativeWindowHandle() const;
    void* getNativeDisplayHandle() const;
#endif

    // Callbacks GLFW
    static void keyCallback        (GLFWwindow*, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow*, int, int, int);
    static void cursorPosCallback  (GLFWwindow*, double, double);
    static void scrollCallback     (GLFWwindow*, double, double);
    static void windowSizeCallback (GLFWwindow*, int, int);

private:
    GLFWwindow* m_window{};
    InputState  m_input{};

#ifdef USE_BGFX
    bgfx::ViewId m_viewId = 0;
    int          m_bgfxWidth{};
    int          m_bgfxHeight{};
#endif
};