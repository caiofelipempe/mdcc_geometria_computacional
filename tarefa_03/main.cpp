#include "renderer.hpp"
#include "input.h"
#include "vector.hpp"
#include "point.hpp"
#include "mesh.hpp"
#include "utils/convex_hull.hpp"

using namespace geometry;

#include <imgui.h>
#include <imgui_internal.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <numbers>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;
class Tarefa03 : public Renderer {
public:
    using Renderer::Renderer;

private:
    int width  = 0;
    int height = 0;

    float dt;

    double mouseX{0.0};
    double mouseY{0.0};
    double mouseDx{0.0};
    double mouseDy{0.0};

    float leftPanelWidth = 300.0f;

    GLuint fbo = 0;
    GLuint fboTexture = 0;
    GLuint rbo = 0;
    ImVec2 canvasSize = {0, 0};
    ImVec2 canvasOrigin = {0, 0};
    bool holdingOnCanvasMouseBtn0 = false;
    bool holdingOnCanvasMouseBtn1 = false;

    int question = 0;

    struct Q2UI {
        enum ButtomClick {
                none = 0,
                hull,
                clear,
        } buttom_click;
    } q2ui; 
    struct Q2State {
        const std::vector<Point2f> polygon {
            {-.5f, -.8f}, 
            {.0f, -.8f}, 
            {.5f, -.8f}, 
            {1.0f, -0.25f}, 
            {.55f, .05f}, 
            {.1f, .1f}, 
            {0.2f, .8f}, 
            {-.5f, .8f},
            {-.4f, .13f},
            {-1.f, -.2f}
        };
        std::vector<Point2f> hull;
    } q2state;

    struct Q3UI {
        enum ButtomClick {
                none = 0,
                random10,
                random100,
                random1000,
                exq2,
                hull,
                clear,
        } buttom_click;
    } q3ui; 
    struct Q3State {
        std::vector<Point2f> polygon;
        std::vector<Point2f> hull;
    } q3state;

    struct Q4UI {
        enum ButtomClick {
                none = 0,
                random10,
                random100,
                random1000,
                exq2,
                hull,
                clear,
        } buttom_click;
    } q4ui; 
    struct Q4State {
        std::vector<Point2f> polygon;
        std::vector<Point2f> hull;
    } q4state;

    struct Q5UI {
        enum ButtomClick {
                none = 0,
                random10,
                random100,
                random1000,
                exq2,
                hull,
                clear,
        } buttom_click;
    } q5ui; 
    struct Q5State {
        std::vector<Point2f> polygon;
        std::vector<Point2f> hull;
    } q5state;

    struct Q6UI {
        enum ButtomClick {
                none = 0,
                hull,
                clear,
        } buttom_click;
    } q6ui; 
    struct Q6State {
        struct Camera {
            float boom = 15;
            float angleX = 0;
            float angleY = 0;
            GLdouble centerX = 0;
            GLdouble centerY = 0;
            GLdouble centerZ = 0;
            GLdouble forwardX = 0;
            GLdouble forwardY = 0;
            GLdouble forwardZ = 0;
            GLdouble upX = 0;
            GLdouble upY = 0;
            GLdouble upZ = 0;
            GLdouble rightX = 0;
            GLdouble rightY = 0;
            GLdouble rightZ = 0;
        } camera;

        const std::vector<Point3f> points {{0.f, 0.f,-5.f}, {2.f, 0.f, 0.f}, {0.f, 3.f, 0.f}, {0.f, -5.f, 0.f}, {-5.f, 0.f, 0.f}, {0.f, 5.f, 0.f}};
        std::optional<Mesh3f> mesh;
    } q6state;

protected:
    void onInit(int w, int h, const std::string&) override {
        onWindowResize(w, h);
    }

    void onUpdate(float dt) override {
        switch (question) {
            case 1: updateQ1(); break;
            case 2: updateQ2(); break;
            case 3: updateQ3(); break;
            case 4: updateQ4(); break;
            case 5: updateQ5(); break;
            case 6: updateQ6(dt); break;
            default: break;
        }
    }

    void onUI() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin(
            "Main",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        float fullHeight = ImGui::GetContentRegionAvail().y;

        ImGui::BeginChild("Panel", ImVec2(leftPanelWidth, fullHeight), true);
        panelUI();
        ImGui::EndChild();

        drawVerticalSplitter(leftPanelWidth, 150.0f, 200.0f);

        ImGui::SameLine();
        ImGui::BeginChild(
            "Canvas",
            ImVec2(0, fullHeight),
            true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );
        drawCanvas();
        ImGui::EndChild();

        ImGui::End();
    }

private:
    void updateQ1() {
        
    }
    
    void updateQ2() {
        auto& ui = q2ui;
        auto& state = q2state;

        switch (ui.buttom_click)
        {
            case Q2UI::ButtomClick::hull:
                state.hull = convex_hull::graham(state.polygon);
                break;
            case Q2UI::ButtomClick::clear:
                state.hull.clear();
                break;
            
            default:
                break;
        }
    }

    void updateQ3() {
        auto& ui = q3ui;
        auto& state = q3state;

        switch (ui.buttom_click)
        {
            case Q3UI::ButtomClick::clear:
                state.hull.clear();
                break;

            case Q3UI::ButtomClick::hull:
                state.hull = convex_hull::jarvis(state.polygon);
                break;

            case Q3UI::ButtomClick::random10:
            case Q3UI::ButtomClick::random100:
            case Q3UI::ButtomClick::random1000:
            {
                state.polygon.clear();
                state.hull.clear();

                int rm = ui.buttom_click == Q3UI::ButtomClick::random10 ? 10 : (ui.buttom_click == Q3UI::ButtomClick::random100 ? 100 : 1000);
                for (int i = 0; i < rm; ++i) {
                    float x = (float)rand() / RAND_MAX; // [0, 1]
                    float y = (float)rand() / RAND_MAX;

                    x = x * 2.0f - 1.0f;
                    y = y * 2.0f - 1.0f;

                    state.polygon.emplace_back(Point2f({x, y}));
                }

            } break;

            case Q3UI::ButtomClick::exq2:
            {
                state.polygon.clear();
                state.hull.clear();

                state.polygon = q2state.polygon;
            } break;
            
            default:
                break;
        }
    }

    void updateQ4() {
        auto& ui = q4ui;
        auto& state = q4state;

        switch (ui.buttom_click)
        {
            case Q4UI::ButtomClick::clear:
                state.hull.clear();
                break;

            case Q4UI::ButtomClick::hull:
                state.hull = convex_hull::quickhull(state.polygon);
                break;

            case Q4UI::ButtomClick::random10:
            case Q4UI::ButtomClick::random100:
            case Q4UI::ButtomClick::random1000:
            {
                state.polygon.clear();
                state.hull.clear();

                int rm = ui.buttom_click == Q4UI::ButtomClick::random10 ? 10 : (ui.buttom_click == Q4UI::ButtomClick::random100 ? 100 : 1000);
                for (int i = 0; i < rm; ++i) {
                    float x = (float)rand() / RAND_MAX; // [0, 1]
                    float y = (float)rand() / RAND_MAX;

                    x = x * 2.0f - 1.0f;
                    y = y * 2.0f - 1.0f;

                    state.polygon.emplace_back(Point2f({x, y}));
                }

            } break;

            case Q4UI::ButtomClick::exq2:
            {
                state.polygon.clear();
                state.hull.clear();

                state.polygon = q2state.polygon;
            } break;
            
            default:
                break;
        }
    }

    void updateQ5() {
        auto& ui = q5ui;
        auto& state = q5state;

        switch (ui.buttom_click)
        {
            case Q5UI::ButtomClick::clear:
                state.hull.clear();
                break;

            case Q5UI::ButtomClick::hull:
                state.hull = convex_hull::mergehull(state.polygon);
                break;

            case Q5UI::ButtomClick::random10:
            case Q5UI::ButtomClick::random100:
            case Q5UI::ButtomClick::random1000:
            {
                state.polygon.clear();
                state.hull.clear();

                int rm = ui.buttom_click == Q5UI::ButtomClick::random10 ? 10 : (ui.buttom_click == Q5UI::ButtomClick::random100 ? 100 : 1000);
                for (int i = 0; i < rm; ++i) {
                    float x = (float)rand() / RAND_MAX; // [0, 1]
                    float y = (float)rand() / RAND_MAX;

                    x = x * 2.0f - 1.0f;
                    y = y * 2.0f - 1.0f;

                    state.polygon.emplace_back(Point2f({x, y}));
                }

            } break;

            case Q5UI::ButtomClick::exq2:
            {
                state.polygon.clear();
                state.hull.clear();

                state.polygon = q2state.polygon;
            } break;
            
            default:
                break;
        }
    }

    void updateQ6(float dt) {
        auto& ui = q6ui;
        auto& state = q6state;

        state.camera.angleX += 20.0f * dt;
        
        if (state.camera.angleX >= 360.0f) {
            state.camera.angleX -= 360.0f;
        }

        switch (ui.buttom_click)
        {
            case Q6UI::ButtomClick::hull:
                state.mesh = convex_hull::giftWrapping(state.points);
                break;
            case Q6UI::ButtomClick::clear:
                state.mesh = std::nullopt;
                break;
            default:
                break;
        }
        ui.buttom_click = Q6UI::ButtomClick::none;
    }

    void panelUIQ1() {
        ImGui::TextWrapped("Se existisse um algoritmo que computasse o fecho convexo em tempo menor que O(n log n), então seria possível ordenar ( n ) números em tempo menor que O(n log n), o que contradiz a cota inferior do problema de ordenação.");
    }

    void panelUIQ2() {
        auto& ui = q2ui;
        auto& state = q2state;

        ui.buttom_click = Q2UI::ButtomClick::none;
        if(state.hull.size() > 0) {
            if(ImGui::Button("Clear")) {
                ui.buttom_click = Q2UI::ButtomClick::clear;
            }
        } else {
            if(ImGui::Button("Grahan")) {
                ui.buttom_click = Q2UI::ButtomClick::hull;
            }
        }
    }

    void panelUIQ3() {
        auto& ui = q3ui;
        auto& state = q3state;

        ui.buttom_click = Q3UI::ButtomClick::none;
        if(ImGui::Button("Random 10")) {
            ui.buttom_click = Q3UI::ButtomClick::random10;
        }
        if(ImGui::Button("Random 100")) {
            ui.buttom_click = Q3UI::ButtomClick::random100;
        }
        if(ImGui::Button("Random 1000")) {
            ui.buttom_click = Q3UI::ButtomClick::random1000;
        }
        if(ImGui::Button("Exemplo da questão 2")) {
            ui.buttom_click = Q3UI::ButtomClick::exq2;
        }
        if(state.polygon.size() > 0){
            if(state.hull.size() > 0) {
                if(ImGui::Button("Clear")) {
                    ui.buttom_click = Q3UI::ButtomClick::clear;
                }
            } else {
                if(ImGui::Button("Jarvis")) {
                    ui.buttom_click = Q3UI::ButtomClick::hull;
                }
            }
        }
    }

    void panelUIQ4() {
        auto& ui = q4ui;
        auto& state = q4state;

        ui.buttom_click = Q4UI::ButtomClick::none;
        if(ImGui::Button("Random 10")) {
            ui.buttom_click = Q4UI::ButtomClick::random10;
        }
        if(ImGui::Button("Random 100")) {
            ui.buttom_click = Q4UI::ButtomClick::random100;
        }
        if(ImGui::Button("Random 1000")) {
            ui.buttom_click = Q4UI::ButtomClick::random1000;
        }
        if(ImGui::Button("Exemplo da questão 2")) {
            ui.buttom_click = Q4UI::ButtomClick::exq2;
        }
        if(state.polygon.size() > 0){
            if(state.hull.size() > 0) {
                if(ImGui::Button("Clear")) {
                    ui.buttom_click = Q4UI::ButtomClick::clear;
                }
            } else {
                if(ImGui::Button("Quickhull")) {
                    ui.buttom_click = Q4UI::ButtomClick::hull;
                }
            }
        }
    }

    void panelUIQ5() {
        auto& ui = q5ui;
        auto& state = q5state;

        ui.buttom_click = Q5UI::ButtomClick::none;
        if(ImGui::Button("Random 10")) {
            ui.buttom_click = Q5UI::ButtomClick::random10;
        }
        if(ImGui::Button("Random 100")) {
            ui.buttom_click = Q5UI::ButtomClick::random100;
        }
        if(ImGui::Button("Random 1000")) {
            ui.buttom_click = Q5UI::ButtomClick::random1000;
        }
        if(ImGui::Button("Exemplo da questão 2")) {
            ui.buttom_click = Q5UI::ButtomClick::exq2;
        }
        if(state.polygon.size() > 0){
            if(state.hull.size() > 0) {
                if(ImGui::Button("Clear")) {
                    ui.buttom_click = Q5UI::ButtomClick::clear;
                }
            } else {
                if(ImGui::Button("Quickhull")) {
                    ui.buttom_click = Q5UI::ButtomClick::hull;
                }
            }
        }
    }

    void panelUIQ6() {
        auto& ui = q6ui;
        auto& state = q6state;

        ui.buttom_click = Q6UI::ButtomClick::none;
        if(state.mesh.has_value()) {
            if(ImGui::Button("Clear")) {
                ui.buttom_click = Q6UI::ButtomClick::clear;
            }
        } else {
            if(ImGui::Button("Gift Wrapping")) {
                ui.buttom_click = Q6UI::ButtomClick::hull;
            }
        }
    }

    void renderCanvasQ1() {

    }

    void renderCanvasQ2() {
        auto& state = q2state;

        glColor3f(1.0f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        for (const auto& p : state.hull) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();

        glPointSize(4.0f);
        glColor3f(1.0f, 1.0f, 0.3f);
        glBegin(GL_POINTS);
        for (const auto& p : state.polygon) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();
    }

    void renderCanvasQ3() {
        auto& state = q3state;

        glColor3f(1.0f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        for (const auto& p : state.hull) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();

        glPointSize(4.0f);
        glColor3f(1.0f, 1.0f, 0.3f);
        glBegin(GL_POINTS);
        for (const auto& p : state.polygon) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();
    }

    void renderCanvasQ4() {
        auto& state = q4state;

        glColor3f(1.0f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        for (const auto& p : state.hull) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();

        glPointSize(4.0f);
        glColor3f(1.0f, 1.0f, 0.3f);
        glBegin(GL_POINTS);
        for (const auto& p : state.polygon) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();
    }

    void renderCanvasQ5() {
        auto& state = q5state;

        glColor3f(1.0f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        for (const auto& p : state.hull) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();

        glPointSize(4.0f);
        glColor3f(1.0f, 1.0f, 0.3f);
        glBegin(GL_POINTS);
        for (const auto& p : state.polygon) {
            glVertex2f(p[0], p[1]);
        }
        glEnd();
    }

    void renderCanvasQ6(int width, int height) {
        auto& state = q6state;
        auto& cam = q6state.camera;

        glViewport(0, 0, canvasSize.x, canvasSize.y);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        float aspect = (float)canvasSize.x / (float)canvasSize.y;
        gluPerspective(45.0, aspect, 0.1, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float theta = cam.angleX * (std::numbers::pi / 180.0f);
        float phi   = cam.angleY * (std::numbers::pi / 180.0f);

        float camX = cam.centerX + cam.boom * cos(phi) * sin(theta);
        float camY = cam.centerY + cam.boom * sin(phi);
        float camZ = cam.centerZ + cam.boom * cos(phi) * cos(theta);

        gluLookAt(camX, camY, camZ, 
                cam.centerX, cam.centerY, cam.centerZ, 
                0.0, 1.0, 0.0);
   
        if(state.mesh.has_value()) {
            auto& mesh = state.mesh.value();

            glDisable(GL_LIGHTING);
            glLineWidth(1.f);
            glColor3f(0.0f, 1.0f, 0.0f);
            
            glBegin(GL_LINES);
            
            auto vertices = mesh.getVertices();
            for (const auto &face : mesh.getFaces()) {
                auto v0 = vertices[face.v0];
                auto v1 = vertices[face.v1];
                auto v2 = vertices[face.v2];
                glVertex3f(v0[0], v0[1], v0[2]);
                glVertex3f(v1[0], v1[1], v1[2]);
                glVertex3f(v1[0], v1[1], v1[2]);
                glVertex3f(v2[0], v2[1], v2[2]);
                glVertex3f(v2[0], v2[1], v2[2]);
                glVertex3f(v0[0], v0[1], v0[2]);
            }
            
            glEnd();
        }


        glPointSize(4.0f); 
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_POINTS);
        for (const auto& p : state.points) {
            glVertex3f(p[0], p[1], p[2]);
        }
        glEnd();
    }

    void drawVerticalSplitter(float& leftWidth, float minLeft, float minRight) {
        ImGui::SameLine();

        ImGui::InvisibleButton("##splitter", ImVec2(6.0f, -1.0f));

        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            leftWidth += delta;
        }

        float total = ImGui::GetContentRegionAvail().x + leftWidth;
        leftWidth = ImClamp(leftWidth, minLeft, total - minRight);
    }

    void panelUI() {
        int oldQuestion = question;
        constexpr const char* QUESTAO_ITEMS[] = {
            "Selecione uma questão:",
            "Questão 1", "Questão 2", "Questão 3",
            "Questão 4", "Questão 5", "Questão 6"
        };
        if (ImGui::Combo("##questao", &question, QUESTAO_ITEMS, IM_ARRAYSIZE(QUESTAO_ITEMS))) {}
        if(question == 0) {
            question = oldQuestion;
        }

        switch (question)
        {
            case 1: panelUIQ1(); break;
            case 2: panelUIQ2(); break;
            case 3: panelUIQ3(); break;
            case 4: panelUIQ4(); break;
            case 5: panelUIQ5(); break;
            case 6: panelUIQ6(); break;
            default: break;
        }
    }

    void drawCanvas() {
        ImVec2 currentOrigin = ImGui::GetCursorScreenPos();
        this->canvasOrigin = currentOrigin;

        ImVec2 currentSize = ImGui::GetContentRegionAvail();

        if (currentSize.x < 1.0f || currentSize.y < 1.0f) return;

        if (currentSize.x != canvasSize.x || currentSize.y != canvasSize.y) {
            canvasSize = currentSize;
            setupFBO((int)canvasSize.x, (int)canvasSize.y);
        }

        renderCanvas((int)canvasSize.x, (int)canvasSize.y);

        ImGui::Image(
            (ImTextureID)(intptr_t)fboTexture, 
            canvasSize, 
            ImVec2(0, 1), 
            ImVec2(1, 0)
        );
    }
    
    void renderCanvas(int w, int h) {
        if (h == 0) h = 1;

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, w, h);

        glDisable(GL_DEPTH_TEST);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        switch (question) {
            case 1: renderCanvasQ1(); break;
            case 2: renderCanvasQ2(); break;
            case 3: renderCanvasQ3(); break;
            case 4: renderCanvasQ4(); break;
            case 5: renderCanvasQ5(); break;
            case 6: renderCanvasQ6(w, h); break;
            default: break;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glPopAttrib();
    }

    void setupFBO(int w, int h) {
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &fboTexture);
            glDeleteRenderbuffers(1, &rbo);
        }

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Erro: FBO incompleto!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Tarefa03 app;
    app.run(800, 600, "Tarefa 02 - Geometria Computacional");
}
