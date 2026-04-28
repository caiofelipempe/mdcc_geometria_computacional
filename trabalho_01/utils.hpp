#pragma once

#include <array>

struct Color3 {
    union {
        struct {
            float r;
            float g;
            float b;
        };

        std::array<float, 3> array;
    };

    // Construtores úteis
    constexpr ColorRGB() : r(0.f), g(0.f), b(0.f) {}
    constexpr ColorRGB(std::array<float, 3> array) : array(array) {}
    constexpr ColorRGB(float r, float g, float b) : r(r), g(g), b(b) {}
};

struct Color4 {
    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        };

        std::array<float, 4> array;
    };

    // Construtores úteis
    constexpr ColorRGBA() : r(0.f), g(0.f), b(0.f), a(0.f) {}
    constexpr ColorRGBA(std::array<float, 4> array) : array(array) {}
    constexpr ColorRGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};