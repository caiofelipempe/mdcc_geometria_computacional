#pragma once

#include "arithmetic.hpp"
#include <span>
#include <initializer_list>

namespace geometry {

/**
 * @brief Classe Vector que utiliza as funções otimizadas de arithmetic.hpp
 * @tparam T Tipo aritmético (float, double, int, etc)
 * @tparam N Dimensão (0 para dinâmico, >0 para fixo)
 */
template <Arithmetic T, std::size_t N>
class Vector {
public:
    // Alias para facilitar o uso interno
    using Storage = ArithmeticVector<T, N>;

    Storage data;

    /* ================= CONSTRUTORES ================= */

    Vector() : data(make_similar<T, N>(data)) {}

    // Construtor para tamanho dinâmico (N=0)
    explicit Vector(std::size_t size) requires (N == 0)
        : data(size) {}

    // Construtor por lista de inicialização
    Vector(std::initializer_list<T> init) {
        if constexpr (N == 0) {
            data.assign(init.begin(), init.end());
        } else {
            if (init.size() != N) throw std::invalid_argument("Dimensão incorreta");
            std::copy(init.begin(), init.end(), data.begin());
        }
    }

    /* ================= ACESSO E UTILITÁRIOS ================= */

    [[nodiscard]] std::size_t size() const noexcept {
        return get_size<T, N>(data);
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
        res.data = geometry::add<T, N>(this->data, rhs.data);
        return res;
    }

    // Subtração: Vector - Vector
    Vector operator-(const Vector& rhs) const {
        Vector res;
        res.data = geometry::sub<T, N>(this->data, rhs.data);
        return res;
    }

    // Multiplicação elemento a elemento
    Vector operator*(const Vector& rhs) const {
        Vector res;
        res.data = make_similar<T, N>(this->data);
        simd::apply_op<simd::Mul>(res.data_ptr(), this->data_ptr(), rhs.data_ptr(), size());
        return res;
    }

    // Escalares
    Vector operator*(T scalar) const {
        Vector res;
        res.data = geometry::mul<T, N>(this->data, scalar);
        return res;
    }

    Vector operator/(T scalar) const {
        Vector res;
        res.data = geometry::div<T, N>(this->data, scalar);
        return res;
    }

    /* ================= ÁLGEBRA LINEAR ================= */

    T dot(const Vector& rhs) const {
        return geometry::dot<T, N>(this->data, rhs.data);
    }

    // Cross Product 2D (Escalar)
    template <std::size_t M = N> requires (M == 2)
    T cross(const Vector& rhs) const {
        return geometry::cross<T, N>(this->data, rhs.data);
    }

    // Cross Product 3D (Vetor)
    template <std::size_t M = N> requires (M == 3)
    Vector cross(const Vector& rhs) const {
        Vector res;
        res.data = geometry::cross<T, N>(this->data, rhs.data);
        return res;
    }

    // Normalização
    Vector normalized() const {
        T d2 = this->dot(*this);
        if (d2 <= T{0}) throw std::runtime_error("Zero norm");
        return (*this) * (static_cast<T>(1) / std::sqrt(d2));
    }

    /* ================= I/O ================= */

    friend std::ostream& operator<<(std::ostream& os, const Vector& v) {
        os << v.data; // Reutiliza o operator<< do arithmetic.hpp
        return os;
    }
};

// Operador para escalar à esquerda: 2.0 * vec
template <Arithmetic T, std::size_t N>
Vector<T, N> operator*(T scalar, const Vector<T, N>& v) {
    return v * scalar;
}

} // namespace geometry