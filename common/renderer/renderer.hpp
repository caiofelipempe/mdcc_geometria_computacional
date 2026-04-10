#pragma once

#include <string>

struct GLFWwindow;
struct InputState;

class Renderer {
public:
    Renderer(int width, int height, const std::string& title);
    virtual ~Renderer();

    void run();

protected:
    virtual void onInit();
    virtual void onUpdate(float dt);
    virtual void onUI();
    virtual void onShutdown();

    const InputState& input() const;

private:
    void initGLFW();
    void initImGui();
    void shutdownImGui();

    // Callbacks GLFW
    static void keyCallback(GLFWwindow*, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow*, int, int, int);
    static void cursorPosCallback(GLFWwindow*, double, double);
    static void scrollCallback(GLFWwindow*, double, double);

private:
    int m_width;
    int m_height;
    std::string m_title;

    GLFWwindow* m_window{};
    InputState* m_input{};
};
``