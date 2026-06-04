<h1 align="center">Questionário do trabalho 3<h1>

<h3><span><b>Nome:</b> Caio Felipe de Moura Peixoto</span>
<span style="float:right"><b>Matrícula:</b> 603198</span><h3>

---

### 1-) Mostre que a cota inferior do fecho convexo é a mesma de ordenação usando redução.

Se existisse um algoritmo que computasse o fecho convexo em tempo menor que **Ω(n log n)**, então seria possível ordenar \( n \) números em tempo menor que **Ω(n log n)**, o que contradiz a cota inferior do problema de ordenação.

---

### 2-) A técnica de Graham encontra o fecho convexo 2D em **O(n log n)**.

#### a) Mostre como é o polígono estrelado dos pontos dados, já ordenados, usando baricentro.

#### b) Iniciando por `p1`, mostre como é o fecho convexo corrente para cada ponto considerado.

#### c) Explique, resumidamente, por que o fecho convexo é **O(n log n)** e não **O(n²)**.

#### d) Implemente o algoritmo de Graham e ache o fecho convexo para os pontos dados. (Observação: p1(y) = p2(y) = p3(y) e p7(y) = p8(y), para os pontos dados abaixo).

---

### 3-) Implemente o algoritmo de fecho convexo por **Jarvis** testando com um exemplo com 10, 100 e 1000 pontos gerados de forma aleatória. Além disso, teste o algoritmo para o exemplo dado na Questão2. Faça uma análise dos resultados obtidos na sua implementação.

---

### 4-) Implemente o algoritmo de fecho convexo por **Quickhull** testando com um exemplo com 10, 100 e 1000 pontos gerados de forma aleatória. Além disso, teste o algoritmo para o exemplo dado na Questão2. Faça uma análise dos resultados obtidos na sua implementação.

---

### 5-) Implemente o algoritmo de fecho convexo por **Mergehull** testando com um exemplo com 10, 100 e 1000 pontos gerados de forma aleatória. Além disso, teste o algoritmo para o exemplo dado na Questão2. Faça uma análise dos resultados obtidos na sua implementação.

---

### 6-) Seja o conjunto C de pontos dado por C = {p1, p2, p3, p4, p5, p6} onde os pontos são dados pelas coordenadas p1=(0;0;-5), p2=(2;0;0), p3=(0;3;0), p4=(0;-5;0), p5=(-5;0;0), p6=(0;5;0).

#### a) Ache uma face inicial que pertença ao fecho convexo desses pontos.

#### b) Mostre, passo a passo, como se forma o fecho convexo usando a técnica do embrulho para presente, incluindo cálculos.

#### c) Qual é o poliedro que representa esse fecho convexo?

#### d) Implemente o algoritmo de embrulho e teste para os pontos dados.