**Questão 1:**\
Seja o pseudo-ângulo orientado q(a, b) o comprimento do arco medido sobre o quadrado unitário e orientado de a (vetor a) para b (vetor b) (onde 0 £ q(a, b) < 8). Pede-se:\
**a)** Exprima q(a, b) em termos de q(a) e q(b). Implemente também esse ângulo.\
`q(a, b) pode ser expresso como q(b) - q(a). Se o valor for negativo, deve-se fazer 8 + q(b) - q(a) para ter o ângulo no intervalo [0, 8).`\
**b)** Diga como usar q(a, b) para decidir se a está à esquerda ou direita de b.\
`Se o valor for maior do que 4, a estará à esquerda de b. Se for menor, estará à direita.`\
**c)** Mostre que se a e b são ortogonais, com a à esquerda de b, então q(a, b) = 6.\
`Se os vetores são ortogonais, então q(a, b) deve ser 2 ou 6. Para a estar à esquerda de b, o valor deve ser maior do que 4, descartando então o valor 2.`\
**Questão 2:**\
Implemente outros tipos de pseudo-ângulos, baseados em cossenos ou não e compare os resultados com o pseudo-ângulo da questào anterior,analisando os resultados obtidos. Para essa comparação tente ordenar um conjunto de ângulos e compare os resultados obtidos.\
**Questão 3:**\
Implemente todas as operações com vetores, desde soma até produto escalar, testando com vetores escolhidos aleatoriamente (inclua casos patológicos como (vetores colineares, etc)).\
**Questão 4:**\
Implemente produto vetorial e teste com vetores escolhidos aleatoriamente. Implemente também interseção de segmentos e área orientada usando o produto vetorial implementado.\
**Questão 5:**\
Existem várias técnicas para ponto em polígono. Implemente os algoritmos do tiro e do índice de rotação, testando para polígonos aleatórios (considere todos os casos patológicos).\
**Questão 6:**\
Seja o polígono dado pelos pontos abaixo, pede-se:
**a)** Mostre como é o polígono estrelado dos pontos dados, já ordenados.
**b)** Considere o ponto no centroide e mostre se ele está dentro do polígono pelo tiro e
índice de rotação, explicando todos os cálculos feitos para esse problema em questão.
**c)** Explique, resumidamente, qual a complexidade de cada um dos algoritmos.
(Observação: p1(y) = p2(y) = p3(y) e p7(y) = p8(y), para os pontos dados abaixo).