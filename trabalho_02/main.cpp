#include "renderer.hpp"
#include "input.h"
#include "geometry.hpp"

using namespace geometry;

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <numbers>
#include <random>

using Point2f = geometry::Point2<float>;
using Segment2f = geometry::Segment<float, 2>;
using Polygonf = geometry::Polygon<float>;
using ColorRGB = Vec<float, 3>;

// ─────────────────────────────────────────────
//  Constantes globais
// ─────────────────────────────────────────────
static constexpr int   PANEL_WIDTH = 200;
static constexpr float PI = std::numbers::pi_v<float>;

// ─────────────────────────────────────────────
//  Enum de questões
// ─────────────────────────────────────────────
enum class Questao {
    Q1 = 0,
    Q2,
    Q3,
    Q4,
    Q5,
    Q6,
    COUNT
};

// ─────────────────────────────────────────────
//  Estado por questão
// ─────────────────────────────────────────────
struct Q1State {
    Point2f vec0{};
    Point2f vec1{};
};

struct Q2State {
    Point2f vec{};
};

struct Q3State {
    enum Op { SOMA = 0, SUBTRACAO, CROSS, DOT, COUNT };
    int   selectedOp = SOMA;
    Point2f  vec0{};
    Point2f  vec1{};
    Point2f  vecr{};
    float prodr = 0.0f;
};

struct Q4State {};

struct Q5State {};
struct Q6State {
    enum Method { RAYCAST, WINDING };
    Method method = RAYCAST;
    static inline constexpr const char* METHOD_ITEMS[] = {
        "Tiro", "Rotação"
    };
    int m_methodComboIndex = 0;
    float maxLength;
    Point2f point{};
    Polygonf poligon{
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
    bool isInside;

    Segment2f ray;
    std::vector<Point2f> interseptionResult;
};

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
class Trabalho02 : public Renderer {
public:
    using Renderer::Renderer;

private:
    int m_width  = 800;
    int m_height = 600;
    int m_canvasWidth;
    int m_canvasHeight;

    static inline constexpr const ColorRGB BLACK = {0, 0, 0};
    static inline constexpr const ColorRGB RED = {1, 0, 0};
    static inline constexpr const ColorRGB GREEN = {0, 1, 0};
    static inline constexpr const ColorRGB BLUE = {0, 0, 1};
    static inline constexpr const ColorRGB YELLOW = {1, 1, 0};
    static inline constexpr const ColorRGB CYAN = {0, 1, 1};
    static inline constexpr const ColorRGB MAGENTA = {1, 0, 1};
    static inline constexpr const ColorRGB WHITE = {1, 1, 1};

    Questao m_questao = Questao::Q1;
    int m_questaoComboIndex = 0;

    static inline constexpr const char* QUESTAO_ITEMS[] = {
        "Questão 1", "Questão 2", "Questão 3",
        "Questão 4", "Questão 5", "Questão 6"
    };

    static inline constexpr const char* Q3_OP_ITEMS[] = {
        "Soma", "Subtração", "Produto Vetorial", "Produto Escalar"
    };

    Q1State m_q1;
    Q2State m_q2;
    Q3State m_q3;
    Q4State m_q4;
    Q5State m_q5;
    Q6State m_q6;

    std::mt19937 m_rng{ std::random_device{}() };
    std::uniform_real_distribution<float> m_dist{ -1.0f, 1.0f };

protected:
    void onInit(int w, int h, const std::string&) override {
        onWindowResize(w, h);
    }

    void onWindowResize(int w, int h) override {
        m_width = w;
        m_height = h;
        onResize();
    }

    void onResize() {
        m_canvasWidth = m_width - PANEL_WIDTH;
        m_canvasHeight = m_height;
        glViewport(PANEL_WIDTH, 0, m_canvasWidth, m_canvasHeight);
    }

    void onUpdate(float) override {
        switch (m_questao) {
        case Questao::Q1: renderQ1(); break;
        case Questao::Q2: renderQ2(); break;
        case Questao::Q3: {
                updateQ3();
                renderQ3();
            } break;
        case Questao::Q6: {updateQ6(); renderQ6();} break;
        default: break;
        }
    }

    void onUI() override {
#ifndef NDEBUG
        ImGui::Begin("Debug");
        ImGui::Text("Mouse: %.1f %.1f", input().mouseX, input().mouseY);
        ImGui::End();
#endif

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, m_height));
        ImGui::Begin("##panel", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("Selecione a questão:");
        if (ImGui::Combo("##questao", &m_questaoComboIndex,
                         QUESTAO_ITEMS, IM_ARRAYSIZE(QUESTAO_ITEMS))) {
            m_questao = static_cast<Questao>(m_questaoComboIndex);
        }

        ImGui::Separator();

        switch (m_questao) {
        case Questao::Q1: uiQ1(); break;
        case Questao::Q2: uiQ2(); break;
        case Questao::Q3: uiQ3(); break;
        case Questao::Q6: uiQ6(); break;
        default:{
            ImGui::TextDisabled("(não implementado)");
        } break;
        }

        ImGui::End();
    }

private:
    // ──────────────── Lógica ────────────────
    void updateQ3() {
        switch (m_q3.selectedOp) {
        case Q3State::SOMA:
            m_q3.vecr = m_q3.vec0 + m_q3.vec1;
            break;
        case Q3State::SUBTRACAO:
            m_q3.vecr = m_q3.vec0 - m_q3.vec1;
            break;
        case Q3State::CROSS:
            m_q3.prodr = geometry::cross(m_q3.vec0, m_q3.vec1);
            break;
        case Q3State::DOT:
            m_q3.prodr = geometry::dot(m_q3.vec0, m_q3.vec1);
            break;
        default: break;
        }
    }

    void updateQ6() {
        m_q6.maxLength = std::min(m_canvasWidth, m_canvasHeight)*0.7f;
        if(input().mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
            // Converte coordenadas de tela para coordenadas de mundo
            float x = ((input().mouseX - PANEL_WIDTH) / (float)m_canvasWidth * 2.0f - 1.0f)/0.7f;
            float y = ((m_height - input().mouseY) / (float)m_canvasHeight * 2.0f - 1.0f)/0.7f;
            if(x > -1.f/0.7f && x < 1.f/0.7f && y > -1.f/0.7f && y < 1.f/0.7f)
            m_q6.point = {x, y};
        }

        if(m_q6.method == Q6State::RAYCAST) {
            m_q6.ray = { m_q6.point, Point2f{1.f/0.7, m_q6.point[1]} };
            
            m_q6.interseptionResult = geometry::segmentPolygonIntersectionPoints(m_q6.ray, m_q6.poligon, 0.0f);
            m_q6.isInside = geometry::isPointInsidePolygonRaycast(m_q6.point, m_q6.poligon);
        } else {
            m_q6.isInside = geometry::isPointInsidePolygonWinding(m_q6.point, m_q6.poligon);
        }
    }

    // ──────────────── Render ────────────────
    bool isCanvasInvalid() {
        return m_canvasHeight <= 0 || m_canvasWidth <= 0;
    }

    void drawPoint(const Point2f& p, float size, const ColorRGB color = WHITE) {
        glPointSize(size);
        glColor3f(color[0], color[1], color[2]);

        glBegin(GL_POINTS);
            glVertex2f(p[0]/m_canvasWidth, p[1]/m_canvasHeight);
        glEnd();
    }

    void drawAxes(const ColorRGB color = WHITE) {
        if (isCanvasInvalid()) return;

        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_LINES);
        glVertex2f(-1, 0); glVertex2f(1, 0);
        glVertex2f(0, -1); glVertex2f(0, 1);
        glEnd();
    }

    void drawVectorLine(const Point2f& v, const ColorRGB color = WHITE) {
        if (isCanvasInvalid()) return;

        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(v[0]/m_canvasWidth, v[1]/m_canvasHeight);
        glEnd();
    }
    
    void drawSegment(const Segment2f& s, const ColorRGB color = WHITE) {
        if (isCanvasInvalid()) return;

        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_LINES);
        glVertex2f(s[0][0]/m_canvasWidth, s[0][1]/m_canvasHeight);
        glVertex2f(s[1][0]/m_canvasWidth, s[1][1]/m_canvasHeight);
        glEnd();
    }

    void drawCircle(float r, const ColorRGB color = WHITE) {
        auto maxLength = std::min(m_canvasWidth, m_canvasHeight);
        if (isCanvasInvalid()) return;
        
        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 40; ++i) {
            float t = 2.0f * PI * i / 40.0f;
            glVertex2f(std::cos(t) * r/m_canvasWidth, std::sin(t) * r/m_canvasHeight);
        }
        glEnd();
    }

    void renderQ1() {
        drawAxes();
        auto maxLength = std::min(m_canvasWidth, m_canvasHeight);
        drawVectorLine(m_q1.vec0*(float)maxLength, GREEN);
        drawVectorLine(m_q1.vec1*(float)maxLength, BLUE);
    }

    void renderQ2() {
        drawAxes();
        auto maxLength = std::min(m_canvasWidth, m_canvasHeight);
        drawVectorLine(m_q2.vec*(float)maxLength, GREEN);
    }

    void renderQ3() {
        drawAxes();
        auto maxLength = std::min(m_canvasWidth, m_canvasHeight)/2;
        drawVectorLine(m_q3.vec0*(float)maxLength, GREEN);
        drawVectorLine(m_q3.vec1*(float)maxLength, BLUE);

        if (m_q3.selectedOp <= Q3State::SUBTRACAO)
            drawVectorLine(m_q3.vecr*(float)maxLength, YELLOW);
        else
            drawCircle(m_q3.prodr*(float)maxLength, MAGENTA);
    }

    void renderQ6() {

        for(int i = 0; i < m_q6.poligon.size(); i++) {
            drawSegment({m_q6.poligon[i]*(float)m_q6.maxLength, m_q6.poligon[(i + 1) % m_q6.poligon.size()]*(float)m_q6.maxLength}, WHITE);
        }

        if(m_q6.method == Q6State::RAYCAST) drawSegment({m_q6.ray[0]*m_q6.maxLength, m_q6.ray[1]*m_q6.maxLength}, BLUE);
        drawPoint(m_q6.point*m_q6.maxLength, 5, m_q6.isInside ? ColorRGB({.3, 1., .3}) : ColorRGB({1., .3, .3}));

        for(int i = 0; i < m_q6.poligon.size(); i++) {
            drawPoint(Point2f({m_q6.poligon[i][0]*(float)m_q6.maxLength, m_q6.poligon[i][1]*(float)m_q6.maxLength}), 5, ColorRGB({1., 1., .3}));
        }

        if(m_q6.method == Q6State::RAYCAST) {
            for (const auto& p : m_q6.interseptionResult) {
                drawPoint(Point2f({p[0]*(float)m_q6.maxLength, p[1]*(float)m_q6.maxLength}), 5, BLUE);
            }
        }
    }

    // ──────────────── UI ────────────────
    void uiQ1() {
        auto pa0 = geometry::pseudoangle(m_q1.vec0);
        auto pa1 = geometry::pseudoangle(m_q1.vec1);

        ImGui::Text("Vetor 1:");
        ImGui::DragFloat2("##v0", &m_q1.vec0[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec0);
        if (pa0) ImGui::Text("Pseudoângulo: %.4f", pa0.value());

        ImGui::Separator();
        ImGui::Text("Vetor 2:");
        ImGui::DragFloat2("##v1", &m_q1.vec1[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec1);
        if (pa1) ImGui::Text("Pseudoângulo: %.4f", pa1.value());
    }

    void uiQ2() {
        ImGui::Text("Vetor:");
        ImGui::DragFloat2("##v", &m_q2.vec[0], 0.01f, -1, 1);
        warnIfZero(m_q2.vec);

        auto pa = geometry::pseudoangle(m_q2.vec);
        auto pb = geometry::pseudoangleAlt(m_q2.vec);

        if (pa) {
            ImGui::Text("Octante: %.4f", pa.value());
            ImGui::Text("Quadrante: %.4f", pb.value());
        }
    }

    void uiQ3() {
        ImGui::Combo("Operação", &m_q3.selectedOp,
                     Q3_OP_ITEMS, IM_ARRAYSIZE(Q3_OP_ITEMS));
        ImGui::Separator();

        ImGui::DragFloat2("Vetor A", &m_q3.vec0[0], 0.01f, -1, 1);
        ImGui::DragFloat2("Vetor B", &m_q3.vec1[0], 0.01f, -1, 1);

        if (ImGui::Button("Aleatório")) {
            m_q3.vec0 = { m_dist(m_rng), m_dist(m_rng) };
            m_q3.vec1 = { m_dist(m_rng), m_dist(m_rng) };
        }

        ImGui::Separator();
        if (m_q3.selectedOp == Q3State::CROSS || m_q3.selectedOp == Q3State::DOT)
            ImGui::Text("Resultado: %.4f", m_q3.prodr);
        else
            ImGui::Text("Resultado: (%.4f, %.4f)", m_q3.vecr[0], m_q3.vecr[1]);
    }

    void uiQ6() {
        ImGui::Text("Selecione o método:");
        if (ImGui::Combo("##method", &m_q6.m_methodComboIndex,
                     m_q6.METHOD_ITEMS, IM_ARRAYSIZE(m_q6.METHOD_ITEMS))) {
            m_q6.method = static_cast<Q6State::Method>(m_q6.m_methodComboIndex);
        }

        ImGui::Text("Ponto:");
        ImGui::DragFloat2("Vetor A", &m_q6.point[0], 0.01f, -1, 1);

        ImGui::Separator();

        ImGui::TextWrapped("Pontos:\np1: (%2.2f, %2.2f)\np2: (%2.2f, %2.2f)\np3: (%2.2f, %2.2f)\np4: (%2.2f, %2.2f)\np5: (%2.2f, %2.2f)\np6: (%2.2f, %2.2f)\np7: (%2.2f, %2.2f)\np8: (%2.2f, %2.2f)\np9: (%2.2f, %2.2f)\np10: (%2.2f, %2.2f)",
            m_q6.poligon[0][0], m_q6.poligon[0][1],
            m_q6.poligon[1][0], m_q6.poligon[1][1],
            m_q6.poligon[2][0], m_q6.poligon[2][1],
            m_q6.poligon[3][0], m_q6.poligon[3][1],
            m_q6.poligon[4][0], m_q6.poligon[4][1],
            m_q6.poligon[5][0], m_q6.poligon[5][1],
            m_q6.poligon[6][0], m_q6.poligon[6][1],
            m_q6.poligon[7][0], m_q6.poligon[7][1],
            m_q6.poligon[8][0], m_q6.poligon[8][1],
            m_q6.poligon[9][0], m_q6.poligon[9][1]
        );
        
        ImGui::Separator();

        m_q6.isInside ?  ImGui::TextColored({0.4f, 1, 0.4f, 1}, "Dentro do poliedro") : ImGui::TextColored({1, 0.4f, 0.4f, 1}, "Fora do poliedro");

        if(m_q6.method == Q6State::RAYCAST) {
            if(m_q6.interseptionResult.empty()) {
                ImGui::Text("Sem interseção");
            } else {
                ImGui::Text("Pontos de colisão:");
                for (size_t i = 0; i < m_q6.interseptionResult.size(); ++i) {
                    ImGui::Text("p%d: (%2.2f, %2.2f)", (int)i, m_q6.interseptionResult[i][0], m_q6.interseptionResult[i][1]);
                }
            }
        }
    }

    static void warnIfZero(const Point2f& v) {
        if (v[0] == 0.0f && v[1] == 0.0f)
            ImGui::TextColored({1, 0.4f, 0.4f, 1}, "Vetor nulo");
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho02 app;
    app.run(800, 600, "Trabalho 02 - Geometria Computacional");
}
