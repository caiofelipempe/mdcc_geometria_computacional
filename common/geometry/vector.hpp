#pragma once

#include "arithmetic.hpp"
#include <span>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>

namespace geometry {

/**
 * @brief Classe Vector que utiliza as funções otimizadas de arithmetic.hpp
 * @tparam T Tipo aritmético (float, double, int, etc)
 * @tparam N Dimensão (0 para dinâmico, >0 para fixo)
 */
template <Arithmetic T, std::size_t N>
class Vector {
public:
    using Storage = ArithmeticVector<T, N>;
    Storage data;

    /* ================= CONSTRUTORES ================= */

    Vector() {
        if constexpr (N == 0) {
            // Para vetores dinâmicos, inicializa vazio ou com zeros se necessário
        } else {
            data.fill(T{0});
        }
    }

    // Construtor para tamanho dinâmico (N=0)
    explicit Vector(std::size_t size) requires (N == 0)
        : data(size, T{0}) {}

    // Construtor por lista de inicialização
    Vector(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N) throw std::invalid_argument("Dimensao incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    /* ================= ACESSO E UTILITÁRIOS ================= */

    [[nodiscard]] std::size_t size() const noexcept {
        return geometry::getSize<T, N>(data);
    }

    T* data_ptr() noexcept { return data.data(); }
    const T* data_ptr() const noexcept { return data.data(); }

    T& operator[](std::size_t i) { return data[i]; }
    const T& operator[](std::size_t i) const { return data[i]; }

    std::span<T> span() noexcept { return {data.data(), size()}; }

    /* ================= OPERAÇÕES DELEGADAS ================= */

    // Soma: Vector + Vector
    Vector operator+(const Vector& rhs) const {
        Vector res;
        if constexpr (N == 0) res.data.resize(size());
        // 'template' é necessário para o compilador não confundir < com 'menor que'
        res.data = geometry::template operator+<T, N>(this->data, rhs.data);
        return res;
    }

    // Subtração: Vector - Vector
    Vector operator-(const Vector& rhs) const {
        Vector res;
        if constexpr (N == 0) res.data.resize(size());
        res.data = geometry::template operator-<T, N>(this->data, rhs.data);
        return res;
    }

    // Multiplicação elemento a elemento
    Vector operator*(const Vector& rhs) const {
        Vector res;
        if constexpr (N == 0) res.data.resize(size());
        res.data = geometry::template operator*<T, N>(this->data, rhs.data);
        return res;
    }

    // Escalares
    Vector operator*(T scalar) const {
        Vector res;
        if constexpr (N == 0) res.data.resize(size());
        res.data = geometry::template mul<T, N>(this->data, scalar);
        return res;
    }

    Vector operator/(T scalar) const {
        // Implementado como multiplicação pelo inverso para usar as otimizações de mul
        return (*this) * (static_cast<T>(1) / scalar);
    }

    /* ================= ÁLGEBRA LINEAR ================= */

    T dot(const Vector& rhs) const {
        return geometry::template dot<T, N>(this->data, rhs.data);
    }

/* ================= ÁLGEBRA LINEAR ================= */

    // Cross Product 2D (Retorna Escalar: x1*y2 - x2*y1)
    template <std::size_t M = N> 
    requires (M == 2)
    T cross(const Vector& rhs) const {
        // Acessa via data[0] e data[1]
        return this->data[0] * rhs.data[1] - this->data[1] * rhs.data[0];
    }

    // Cross Product 3D (Retorna Vetor)
    template <std::size_t M = N> 
    requires (M == 3)
    Vector cross(const Vector& rhs) const {
        Vector res;
        // Assume que geometry::cross<T, 3> está definido em arithmetic.hpp
        res.data = geometry::template cross<T, 3>(this->data, rhs.data);
        return res;
    }

    T sqrNorm() const {
        return this->dot(*this);
    }

    T norm() const {
        return std::sqrt(sqrNorm());
    }

    Vector normalized() const {
        T n = norm();
        if (n <= T{0}) throw std::runtime_error("Zero norm");
        return (*this) * (static_cast<T>(1) / n);
    }

    /* ================= ROTAÇÃO ================= */

    // Rotação 3D usando Quaternions (Vector de 4 posições)
    template <std::size_t M = N> requires (M == 3)
    Vector rotated(const Vector<T, 4>& q) const {
        Vector res;
        res.data = geometry::template rotateWithQuaternion<T>(this->data, q.data);
        return res;
    }

    /* ================= I/O ================= */

    friend std::ostream& operator<<(std::ostream& os, const Vector& v) {
        os << "[";
        for (std::size_t i = 0; i < v.size(); ++i) {
            os << v.data[i] << (i == v.size() - 1 ? "" : ", ");
        }
        os << "]";
        return os;
    }
};

/* ================= OPERADORES GLOBAIS ================= */

template <Arithmetic T, std::size_t N>
Vector<T, N> operator*(T scalar, const Vector<T, N>& v) {
    return v * scalar;
}

/* ================= ALIASES ================= */

template <Arithmetic T> using Vec2 = Vector<T, 2>;
template <Arithmetic T> using Vec3 = Vector<T, 3>;
template <Arithmetic T> using Quat = Vector<T, 4>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Quatf = Quat<float>;
using Quatd = Quat<double>;

} // namespace geometry