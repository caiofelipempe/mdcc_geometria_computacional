#include "renderer.hpp"
#include "input.h"
#include "geometry.hpp"

using namespace geometry;

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <numbers>
#include <random>
#include <iostream>
#include <fstream>
#include <string>


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

struct Q4State {
    Point2f  vec0{};
    Point2f  vec1{};
    float prodr = 0.0f;
};

struct Q5State {
        enum Method { RAYCAST, WINDING };
    Method method = RAYCAST;
    static inline constexpr const char* METHOD_ITEMS[] = {
        "Tiro", "Rotação"
    };
    int m_methodComboIndex = 0;
    float maxLength;
    Point2f point{};
    bool isInside;
    int randomPoints = 15;
    Polygon<float, 0> polygon = generateRandomPolygon<float, 0>(randomPoints);

    Segment2f ray;
    std::vector<Point2f> interseptionResult;
};

struct Q6State {
    enum Method { RAYCAST, WINDING };
    Method method = RAYCAST;
    static inline constexpr const char* METHOD_ITEMS[] = {
        "Tiro", "Rotação"
    };
    int m_methodComboIndex = 0;
    float maxLength;
    Point2f point{};
    static const Polygonf polygon{
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
        case Questao::Q4: {
                updateQ4();
                renderQ4();
            } break;
        case Questao::Q5: {updateQ5(); renderQ5();} break;
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
        case Questao::Q4: uiQ4(); break;
        case Questao::Q5: uiQ5(); break;
        case Questao::Q6: uiQ6(); break;
        default:{
            ImGui::TextDisabled("(não implementado)");
        } break;
        }

        ImGui::End();
    }

private:
    // ──────────────── Lógica ────────────────
    std::string gerarNomeComTimestamp(const std::string& titulo, const std::string& extensao) {
        std::time_t agora = std::time(nullptr);
        std::tm tm = *std::localtime(&agora);

        std::ostringstream oss;
        oss << titulo << "_"
            << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
            << extensao;

        return oss.str();
    }


    void generateTests() { 
        std::string folderName = "testes_resultado_" + gerarTimestamp();
        fs::create_directory(folderName);

        std::string fileName1 = folderName + "/teste1.txt";
        std::ofstream arquivo1(fileName1);
        arquivo1 << "Comprimento do arco medido sobre o quadrado unitário e orientado de dois vetores\n" << std::endl;
        for(int i = 0, i < 100; i++) {
            Point2f p1 = generateRandomPoint(-1000.f, 1000.f);
            Point2f p2 = generateRandomPoint(-1000.f, 1000.f);
            auto a1 = pseudoangleOctante(p1);
            auto a2 = pseudoangleOctante(p2);

            arquivo1 << "Vec 1 :(" << p1[0] << ", " << p1[1] << ")" << std::endl;
            arquivo1 << "Vec 2 :(" << p2[0] << ", " << p2[1] << ")" << std::endl;
            arquivo1 << "Diferença de ângulos: " << a2 << " - " << a1 << " = " << a2 - a1 << std::endl;
        }
        arquivo1.close();

        std::string fileName2 = folderName + "/teste2.txt";
        std::ofstream arquivo2(fileName2);
        arquivo2 << "Comparação de algoritmos de psudoângulo\n" << std::endl;
        for(int i = 0, i < 100; i++) {
            Point2f p = generateRandomPoint(-1000.f, 1000.f);
            auto a8 = pseudoangleOctante(p);
            auto a4 = pseudoangleQuadrant(p);

            arquivo2 << "Vec :(" << p[0] << ", " << p[1] << ")" << std::endl;
            arquivo2 << "Octante: " << a8 << std::endl;
            arquivo2 << "Quadrante: " << a4 << std::endl;
        }
        arquivo2.close();
        
        std::string fileName3 = folderName + "/teste3.txt";
        std::ofstream arquivo3(fileName3);
        arquivo3 << "Operações de adição, subtração, produto escalar e produto vetorial\n" << std::endl;
        for(int i = 0, i < 100; i++) {
            Point2f p1 = generateRandomPoint(-1000.f, 1000.f);
            Point2f p2 = generateRandomPoint(-1000.f, 1000.f);
            auto add = p2 = p1;
            auto sub = p2 - p1;
            auto dot = dot(p1, p2);
            auto cross = cross(p1, p2);

            arquivo3 << "Vec 1 :(" << p1[0] << ", " << p1[1] << ")" << std::endl;
            arquivo3 << "Vec 2 :(" << p2[0] << ", " << p2[1] << ")" << std::endl;
            arquivo3 << "Soma: (" << add[0] << ", " << add[1] << ")" << std::endl;
            arquivo3 << "Subtração: (" << sub[0] << ", " << sub[1] << ")" << std::endl;
            arquivo3 << "Produto escalar: " << dot << std::endl;
            arquivo3 << "Produto vetorial: (" << cross[0] << ", " << cross[1] << ")" << std::endl;
        }
        arquivo3.close();
        
        std::string fileName4 = folderName + "/teste4.txt";
        std::ofstream arquivo4(fileName4);
        arquivo4 << "Interseção de segmentos e área orientada\n" << std::endl;
        for(int i = 0, i < 100; i++) {
            Point2f p11 = generateRandomPoint(-1000.f, 1000.f);
            Point2f p12 = generateRandomPoint(-1000.f, 1000.f);
            Point2f p21 = generateRandomPoint(-1000.f, 1000.f);
            Point2f p22 = generateRandomPoint(-1000.f, 1000.f);

            T o1 = orientedArea2(Triangle{p11, p12, p21});
            T o2 = orientedArea2(Triangle{p11, p12, p22});
            T o3 = orientedArea2(Triangle{p21, p22, p11});
            T o4 = orientedArea2(Triangle{p21, p22, p12});

            auto interseptionResult = interseptionResult(Segment2f{p11, p22}, Segment2f{p21, p22});

            arquivo4 << "Seg 1 :{(" << p11[0] << ", " << p11[1] << "), (" << p12[0] << ", " << p12[0] << ")}" << std::endl;
            arquivo4 << "Seg 2 :{(" << p21[0] << ", " << p21[1] << "), (" << p22[0] << ", " << p22[0] << ")}" << std::endl;
            arquivo4 << "Área orientada 1 : " << o1 << std::endl;
            arquivo4 << "Área orientada 2 : " << o2 << std::endl;
            arquivo4 << "Área orientada 3 : " << o3 << std::endl;
            arquivo4 << "Área orientada 4 : " << o4 << std::endl;
            arquivo4 << "Tem interseção : " << interseptionResult ? "Simm" : "Não" << std::endl;
        }
        arquivo4.close();

        std::string fileName5 = folderName + "/teste5.txt";
        std::ofstream arquivo5(fileName5);
        arquivo5 << "Checagem de ponto dentro de poligono randômico\n" << std::endl;
        Poligonf Polygon<float, 10> polygon = generateRandomPolygon<float, 10>(10);
        arquivo5 << "Vértices do Polígono: \n";
        for(int i = 0; i < poly.size(); i++) {
            arquivo5 << "(" << poly[i] << ", " << poly[i] << "); ";
        }
        arquivo5 << "\n";
        for(int i = 0, i < 100; i++) {
            Point2f p = generateRandomPoint(-1000.f, 1000.f);
            auto isInsideRaycas = isPointInsidePolygonRaycast(p, poly);
            auto isInsideWinding = isPointInsidePolygonWinding(p, poly);

            arquivo5 << "Ponto :(" << p[0] << ", " << p[1] << ")" << std::endl;
            arquivo5 << "Método do tiro: " << isInsideRaycas ? "Dentro" : "Fora" << std::endl;
            arquivo5 << "Método Winding: " << isInsideWinding ? "Dentro" : "Fora" << std::endl;
        }
        arquivo5.close();

        std::string fileName6 = folderName + "/teste6.txt";
        std::ofstream arquivo4(fileName6);
        arquivo6 << "Checagem de ponto dentro de poligono proposto pela questão 6\n" << std::endl;
        auto polygon = m_q6.polygon;
        arquivo6 << "Vértices do Polígono: \n";
        for(int i = 0; i < poly.size(); i++) {
            arquivo6 << "(" << poly[i] << ", " << poly[i] << "); ";
        }
        arquivo6 << "\n";
        for(int i = 0, i < 100; i++) {
            Point2f p = generateRandomPoint(-1000.f, 1000.f);
            auto isInsideRaycas = isPointInsidePolygonRaycast(p, poly);
            auto isInsideWinding = isPointInsidePolygonWinding(p, poly);

            arquivo6 << "Ponto :(" << p[0] << ", " << p[1] << ")" << std::endl;
            arquivo6 << "Método do tiro: " << isInsideRaycas ? "Dentro" : "Fora" << std::endl;
            arquivo6 << "Método Winding: " << isInsideWinding ? "Dentro" : "Fora" << std::endl;
        }
        arquivo6.close();

        std::cout << "Testes executados na pasta: " << folderName << std::endl;
    }

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

    void updateQ4() {
        m_q4.prodr = geometry::cross(m_q4.vec0, m_q4.vec1);
    }

    void updateQ5() {
        m_q5.maxLength = std::min(m_canvasWidth, m_canvasHeight)*0.7f;
        if(input().mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
            // Converte coordenadas de tela para coordenadas de mundo
            float x = ((input().mouseX - PANEL_WIDTH) / (float)m_canvasWidth * 2.0f - 1.0f)/0.7f;
            float y = ((m_height - input().mouseY) / (float)m_canvasHeight * 2.0f - 1.0f)/0.7f;
            if(x > -1.f/0.7f && x < 1.f/0.7f && y > -1.f/0.7f && y < 1.f/0.7f)
            m_q5.point = {x, y};
        }

        if (m_q5.polygon.size() > 3) {    
            if(m_q5.method == Q5State::RAYCAST) {
                m_q5.ray = { m_q5.point, Point2f{1.f/0.7, m_q5.point[1]} };
                
                m_q5.interseptionResult = geometry::segmentPolygonIntersectionPoints(m_q5.ray, m_q5.polygon, 0.0f);
                m_q5.isInside = geometry::isPointInsidePolygonRaycast(m_q5.point, m_q5.polygon);
            } else {
                m_q5.isInside = geometry::isPointInsidePolygonWinding(m_q5.point, m_q5.polygon);
            }
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
            
            m_q6.interseptionResult = geometry::segmentPolygonIntersectionPoints(m_q6.ray, m_q6.polygon, 0.0f);
            m_q6.isInside = geometry::isPointInsidePolygonRaycast(m_q6.point, m_q6.polygon);
        } else {
            m_q6.isInside = geometry::isPointInsidePolygonWinding(m_q6.point, m_q6.polygon);
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
        drawVectorLine(m_q3.vec0*(float)maxLength, YELLOW);
        drawVectorLine(m_q3.vec1*(float)maxLength, BLUE);

        if (m_q3.selectedOp <= Q3State::SUBTRACAO)
            drawVectorLine(m_q3.vecr*(float)maxLength, GREEN);
        else
            drawCircle(m_q3.prodr*(float)maxLength, m_q3.prodr >= 0 ? GREEN : RED);
    }

    void renderQ4() {
        drawAxes();
        auto maxLength = std::min(m_canvasWidth, m_canvasHeight)/2;
        drawVectorLine(m_q4.vec0*(float)maxLength, YELLOW);
        drawVectorLine(m_q4.vec1*(float)maxLength, BLUE);

        drawCircle(m_q4.prodr*(float)maxLength, m_q4.prodr >= 0 ? GREEN : RED);
    }

    void renderQ5() {

        for(int i = 0; i < m_q5.polygon.size(); i++) {
            drawSegment({m_q5.polygon[i]*(float)m_q5.maxLength, m_q5.polygon[(i + 1) % m_q5.polygon.size()]*(float)m_q5.maxLength}, WHITE);
        }

        if(m_q5.method == Q5State::RAYCAST) drawSegment({m_q5.ray[0]*m_q5.maxLength, m_q5.ray[1]*m_q5.maxLength}, BLUE);
        drawPoint(m_q5.point*m_q5.maxLength, 5, m_q5.isInside ? ColorRGB({.3, 1., .3}) : ColorRGB({1., .3, .3}));

        for(int i = 0; i < m_q5.polygon.size(); i++) {
            drawPoint(Point2f({m_q5.polygon[i][0]*(float)m_q5.maxLength, m_q5.polygon[i][1]*(float)m_q5.maxLength}), 5, ColorRGB({1., 1., .3}));
        }

        if(m_q5.method == Q5State::RAYCAST) {
            for (const auto& p : m_q5.interseptionResult) {
                drawPoint(Point2f({p[0]*(float)m_q5.maxLength, p[1]*(float)m_q5.maxLength}), 5, BLUE);
            }
        }
    }

    void renderQ6() {

        for(int i = 0; i < m_q6.polygon.size(); i++) {
            drawSegment({m_q6.polygon[i]*(float)m_q6.maxLength, m_q6.polygon[(i + 1) % m_q6.polygon.size()]*(float)m_q6.maxLength}, WHITE);
        }

        if(m_q6.method == Q6State::RAYCAST) drawSegment({m_q6.ray[0]*m_q6.maxLength, m_q6.ray[1]*m_q6.maxLength}, BLUE);
        drawPoint(m_q6.point*m_q6.maxLength, 5, m_q6.isInside ? ColorRGB({.3, 1., .3}) : ColorRGB({1., .3, .3}));

        for(int i = 0; i < m_q6.polygon.size(); i++) {
            drawPoint(Point2f({m_q6.polygon[i][0]*(float)m_q6.maxLength, m_q6.polygon[i][1]*(float)m_q6.maxLength}), 5, ColorRGB({1., 1., .3}));
        }

        if(m_q6.method == Q6State::RAYCAST) {
            for (const auto& p : m_q6.interseptionResult) {
                drawPoint(Point2f({p[0]*(float)m_q6.maxLength, p[1]*(float)m_q6.maxLength}), 5, BLUE);
            }
        }
    }

    // ──────────────── UI ────────────────
    void uiQ1() {
        auto pa0 = geometry::pseudoangleOctante(m_q1.vec0);
        auto pa1 = geometry::pseudoangleOctante(m_q1.vec1);

        ImGui::Text("Vetor 1:");
        ImGui::DragFloat2("##v0", &m_q1.vec0[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec0);
        if (pa0) ImGui::Text("Pseudoângulo: %.4f", pa0.value());

        ImGui::Separator();
        ImGui::Text("Vetor 2:");
        ImGui::DragFloat2("##v1", &m_q1.vec1[0], 0.01f, -1, 1);
        warnIfZero(m_q1.vec1);
        if (pa1) ImGui::Text("Pseudoângulo: %.4f", pa1.value());
        
        if (pa0 && pa1) {
            ImGui::Separator();
            ImGui::TextWrapped("Angulo entre v1 e v2:\n %.4f", pa1.value() - pa0.value());
        }
    }

    void uiQ2() {
        ImGui::Text("Vetor:");
        ImGui::DragFloat2("##v", &m_q2.vec[0], 0.01f, -1, 1);
        warnIfZero(m_q2.vec);

        auto pa = geometry::pseudoangleOctante(m_q2.vec);
        auto pb = geometry::pseudoangleQuadrant(m_q2.vec);

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

    void uiQ4() {
        ImGui::DragFloat2("Vetor A", &m_q4.vec0[0], 0.01f, -1, 1);
        ImGui::DragFloat2("Vetor B", &m_q4.vec1[0], 0.01f, -1, 1);

        if (ImGui::Button("Aleatório")) {
            m_q4.vec0 = { m_dist(m_rng), m_dist(m_rng) };
            m_q4.vec1 = { m_dist(m_rng), m_dist(m_rng) };
        }

        ImGui::Separator();

        ImGui::Text("Resultado: %.4f", m_q4.prodr);
    }

    void uiQ5() {
        ImGui::Text("Selecione o método:");
        if (ImGui::Combo("##method", &m_q5.m_methodComboIndex,
                     m_q5.METHOD_ITEMS, IM_ARRAYSIZE(m_q5.METHOD_ITEMS))) {
            m_q5.method = static_cast<Q5State::Method>(m_q5.m_methodComboIndex);
        }

        ImGui::Text("Ponto:");
        ImGui::DragFloat2("Vetor A", &m_q5.point[0], 0.01f, -1, 1);

        ImGui::Separator();

        ImGui::DragInt("##randomPoints", &m_q5.randomPoints, 0.01f, 3, 100);
        if (ImGui::Button("Gerar poligono aleatório")) {
            m_q5.polygon = generateRandomPolygon<float, 0>(m_q5.randomPoints);
        }

        ImGui::Separator();

        m_q5.isInside ?  ImGui::TextColored({0.4f, 1, 0.4f, 1}, "Dentro do poliedro") : ImGui::TextColored({1, 0.4f, 0.4f, 1}, "Fora do poliedro");

        if(m_q5.method == Q5State::RAYCAST) {
            if(m_q5.interseptionResult.empty()) {
                ImGui::Text("Sem interseção");
            } else {
                ImGui::Text("Pontos de colisão:");
                for (size_t i = 0; i < m_q5.interseptionResult.size(); ++i) {
                    ImGui::Text("p%d: (%2.2f, %2.2f)", (int)i, m_q5.interseptionResult[i][0], m_q5.interseptionResult[i][1]);
                }
            }
        }

        if(m_q5.polygon.size() > 3){
            ImGui::Separator();
            ImGui::Text("Pontos:\n");
            for (size_t i = 0; i < m_q5.polygon.size(); ++i) {
                ImGui::Text("p%d: (%2.2f, %2.2f)", (int)i, m_q5.polygon[i][0], m_q5.polygon[i][1]);
            }
        }
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
            m_q6.polygon[0][0], m_q6.polygon[0][1],
            m_q6.polygon[1][0], m_q6.polygon[1][1],
            m_q6.polygon[2][0], m_q6.polygon[2][1],
            m_q6.polygon[3][0], m_q6.polygon[3][1],
            m_q6.polygon[4][0], m_q6.polygon[4][1],
            m_q6.polygon[5][0], m_q6.polygon[5][1],
            m_q6.polygon[6][0], m_q6.polygon[6][1],
            m_q6.polygon[7][0], m_q6.polygon[7][1],
            m_q6.polygon[8][0], m_q6.polygon[8][1],
            m_q6.polygon[9][0], m_q6.polygon[9][1]
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
