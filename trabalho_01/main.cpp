#include "renderer.hpp"
#include "input.h"
#include "vector.hpp"
#include "utils.hpp"

using namespace geometry;
using namespace sort;

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

// ─────────────────────────────────────────────
//  Aplicação principal
// ─────────────────────────────────────────────
class Trabalho01 : public Renderer {
    int width  = 800;
    int height = 600;
}

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Trabalho01 app;
    app.run(800, 600, "Trabalho 01 - Geometria Computacional");
}
