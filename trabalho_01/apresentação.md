---
marp: true
---

# Convex Hull 3D com Algoritmo de Força Bruta

### Trabalho de Geometria Computacional

- Linguagem: C++
- Renderização: OpenGL + ImGui
- Estrutura geométrica: Mesh 3D
- Objetivo:
  - Gerar o fecho convexo (Convex Hull) de um conjunto de pontos 3D
  - Visualizar o resultado em tempo real

---

# Objetivo do Projeto

## Objetivos principais

- Carregar modelos `.obj`
- Gerar pontos aleatórios em 3D
- Construir o Convex Hull
- Visualizar:
  - pontos
  - vértices
  - arestas
  - faces
- Navegação interativa com câmera 3D

---

# Tecnologias Utilizadas

## Bibliotecas utilizadas

### Interface
- ImGui

### Janela e Input
- GLFW

### Renderização
- OpenGL
- GLU
- GLEW

### Geometria Computacional
- CGAL

---

# Estrutura Geral do Programa

## Fluxo da aplicação

```text
Usuário
   ↓
Carrega pontos / modelo OBJ
   ↓
Armazena pontos em memória
   ↓
Executa algoritmo Convex Hull
   ↓
Gera Mesh 3D
   ↓
Renderiza faces, arestas e vértices
```

---

# O que é Convex Hull?

## Definição

O Convex Hull é o menor sólido convexo que contém todos os pontos.

### Intuição

Imagine:

- vários pontos no espaço
- uma borracha elástica envolvendo todos eles

A forma resultante é o Convex Hull.

---

# Exemplo Conceitual

## Em 2D

- pontos espalhados
- contorno externo conecta os pontos extremos

## Em 3D

- o hull vira uma casca tridimensional
- composto por triângulos

---

# Estratégia Utilizada

# Algoritmo de Força Bruta

## Ideia principal

Testar TODAS as combinações possíveis de:

```text
3 pontos → possível face
```

Depois verificar:

- todos os outros pontos estão do mesmo lado do plano?

Se sim:

✅ o triângulo pertence ao Convex Hull

---

# Estrutura Principal do Algoritmo

Código principal:

```cpp
for (int i = 0; i < n-2; i++) {
    for (int j = i+1; j < n-1; j++) {
        for (int k = j+1; k < n; k++) {

            auto ss = sameSide(points, i, j, k);

            if (ss < 0)
                faces.push_back(i, j, k);

            else if (ss > 0)
                faces.push_back(k, j, i);
        }
    }
}
```

---

# Passo 1: Escolher 3 Pontos

## Formação de um triângulo candidato

O algoritmo escolhe:

```text
P1, P2, P3
```

Esses pontos formam um plano.

---

# Passo 2: Construir o Plano

## Vetores da face

A partir do ponto base:

```cpp
edge1 = points[b] - points[a]
edge2 = points[c] - points[a]
```

---

# Produto Vetorial

## Normal da face

A normal é obtida por:

```math
\vec{n} = \vec{edge1} \times \vec{edge2}
```

A normal indica:

- orientação do plano
- qual lado é positivo
- qual lado é negativo

---

# Passo 3: Testar Todos os Pontos

## Verificação geométrica

Para cada ponto restante:

```cpp
to_point = points[i] - po;
dist = normal.dot(to_point);
```

---

# Produto Escalar

## Interpretação do sinal

```math
d = \vec{n} \cdot (P - P_0)
```

### Resultado:

- `d > 0`
  - ponto está de um lado

- `d < 0`
  - ponto está do outro lado

- `d = 0`
  - ponto está no plano

---

# Regra do Convex Hull

## Critério de validade

Uma face pertence ao hull SOMENTE se:

✅ todos os pontos estiverem:

- no mesmo lado
OU
- exatamente sobre o plano

Se existir ponto dos dois lados:

❌ a face é descartada

---

# Função sameSide()

## Responsável pela validação

```cpp
int sameSide(...) {
    ...
}
```

Ela retorna:

| Retorno | Significado |
|---|---|
| `0` | pontos em ambos os lados |
| `1` | todos do lado positivo |
| `-1` | todos do lado negativo |

---

# Orientação das Faces

## Correção da ordem dos vértices

Se:

```cpp
ss > 0
```

A ordem dos vértices é invertida:

```cpp
(k, j, i)
```

Isso garante:

- normais consistentes
- iluminação correta
- orientação adequada

---

# Complexidade do Algoritmo

## Custo computacional

### Combinações de faces

```math
\binom{n}{3} = \frac{n!}{3!(n-3)!}
```

### Para cada face:
- testar todos os pontos

### Complexidade final

```math
O(n^4)
```

---

# Vantagens da Força Bruta

## Pontos positivos

✅ Fácil implementação

✅ Fácil entendimento

✅ Ótimo para aprendizado

✅ Funciona bem para poucos pontos

---

# Desvantagens

## Limitações

❌ Muito lento para muitos pontos

❌ Complexidade alta

❌ Não escalável

---

# Renderização do Resultado

## O programa renderiza:

### Faces
```cpp
GL_TRIANGLES
```

### Arestas
```cpp
GL_LINES
```

### Vértices
```cpp
GL_POINTS
```

---

# Recursos da Interface

## Funcionalidades

- câmera orbital
- zoom
- pan
- rotação
- carregamento OBJ
- salvar modelo
- geração aleatória de pontos
- ativar/desativar:
  - faces
  - arestas
  - vértices
  - pontos originais

---

# Iluminação

## Sistema de iluminação OpenGL

Utiliza:

- luz ambiente
- luz difusa
- luz especular

### Resultado

- melhor percepção espacial
- destaque das faces do hull

---

# Comparação com CGAL

## Implementações presentes

### CGAL
```cpp
convex_hull::cgal(...)
```

### Força Bruta
```cpp
convex_hull::bruteForce(...)
```

---

# Diferença Entre Elas

| Força Bruta | CGAL |
|---|---|
| simples | otimizada |
| didática | industrial |
| lenta | rápida |
| O(n⁴) | muito mais eficiente |

---

# Resultado Final

## O sistema consegue

✅ carregar modelos

✅ gerar pontos aleatórios

✅ calcular o Convex Hull

✅ visualizar em 3D

✅ exportar o modelo final

---

# Conclusão

## Conclusão

O trabalho demonstra:

- fundamentos de geometria computacional
- construção de hulls convexos
- álgebra vetorial
- renderização 3D
- manipulação de câmera
- integração OpenGL + ImGui + CGAL

---

# Obrigado!

## Perguntas?