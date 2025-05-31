# Trabalho 2 - AEDS
## Recomendação de filmes

<div align="center">
    <img src="https://img.shields.io/badge/Kaggle-20BEFF?style=for-the-badge&logo=Kaggle&logoColor=white"/>
    <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>
    <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white"/>
    <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black"/>
    <img src="https://img.shields.io/badge/GIT-E44C30?style=for-the-badge&logo=git&logoColor=white"/>
</div>

## IMPORTANTE!

Para rodar o programa, é preciso ter os arquivos movies.csv e ratings.csv dentro da pasta kaggle-data. É necessário baixar esses arquivos disponíveis no link do PDF do trabalho.
Além disso, no arquivo "datasets/explore.dat" devem estar definido em cada linha, o id do usuário que receberá recomendações.

## Introdução

Esse projeto foi desenvolvido como trabalho final da disciplina de Algoritmos e Estruturas de Dados I, ministrada por Michel Pires, do curso de Engenharia de Computação do CEFET-MG (Campus Divinópolis). O objetivo desse trabalho é gerar recomendações de filmes para usuários específicos com base numa base de dados de filmes pública, que possui mais de 25 milhões de registros, contendo identificadores para os usuários e filmes, suas avaliações e o momento em que o registro foi feito. O trabalho foi desenvolvido utilizando as linguagens C e C++, com o grupo sendo composto por 5 pessoas.

## Metodologia

Nesta etapa da documentação, explicaremos a linha de raciocínio utilizada para pensar as funcionalidades e decisões desse programa.

### Pré-processamento

Para essa primeira parte, tentei primeiro fazer uma leitura mais simples utilizando ```std::ifstream, std::getline, std::vector```, sem visar o desempenho do programa. Com essa abordagem inicial, foi possível obter um resultado médio de 75-80 segundos para pré-processar o dados do arquivo ratings.csv.

[Uma versão anterior da função que lida com esses dados pode ser encontrada nesse link](https://github.com/arthur-lage/movie-recommender/commit/303c64bf38c0b991bd4b02f4f14d6b8a00441137#diff-a6be4517d1cc29d972f91468e0137d1ba8455764b0176ff894a05fd4b4e07935R39-L41)

O arquivo era percorrido duas vezes: uma vez para descobrir a quantidade de avaliações feitas por um usuário e a quantidade de avaliações de um filme, e a outra pra ler os dados do filme e salvar numa estrutura de dados.
Após isso, essa estrutura era percorrida e os dados eram escritos no arquivo "input.dat".

Entretanto, essa abordagem, por ler o arquivo mais de uma vez, e salvar os dados de forma custosa, tornava o algoritmo muito demorado.

Por isso, decidimos tentar de outra maneira, com o objetivo de tornar o programa mais rápido.

Assim, ao invés de usar ifstream, usamos agora o FILE* da linguagem C, que apesar de ter uma implementação menos simples, permite com que lidemos com o arquivo de forma bem mais rápida. Além disso, utilizamos vetores de char ao invés de strings, e funções do C, como strlen, strtol, strtof, que tem um desempenho bem maior que as funções de string do C++.
Outra mudança importante foi substituir os vetores por estruturas que permitissem acesso mais rápido, como unordered_map e unordered_set, além de facilitarem a verificação de que os seus registros são únicos.

[Versão atual dessa função](https://github.com/arthur-lage/movie-recommender/blob/2d7e87200a712fd513e8bd70e9829c681560a816/src/input_preprocessor.cpp#L43)

### Constantes

Descrição das constantes definidas no arquivo [config.hpp](https://github.com/arthur-lage/movie-recommender/blob/2d7e87200a712fd513e8bd70e9829c681560a816/include/config.hpp#L1C1-L6C51)

```c
const int MININUM_REVIEW_COUNT_PER_USER = 50;
const int MININUM_REVIEW_COUNT_PER_MOVIE = 50;

const int NUMBER_OF_RECOMMENDATIONS_PER_USER = 10;
```

A primeira constante define a quantidade mínima de avaliações que um usuário deve fazer para que ele seja incluído no "input.dat".

A segunda constante define a quantidade mínima de avaliações que um filme deve possuir para ser incluído no "input.dat".

A terceira constante define a quantidade de recomendações que cada usuário irá receber.

## Métodos de recomendação

Para esse projeto, testamos diferentes métodos de gerar recomendações, com o objetivo de encontrar um que possuísse melhor desempenho para o programa.
Foram testadas as seguintes estratégias:

- Distância euclidiana 
- Similaridade de Cossenos
- Correlação de Pearson
- Jaccard
- Manhattan

Abaixo está uma breve descrição sobre esses métodos, e a performance média que obtivemos nos testes.

### Distância euclidiana

Esse método funciona da seguinte maneira:

- 1. Compara um usuário desejado com os outros com base na nota de seus filmes assistidos em comum.
- 2. Avalia quais são os usuários com notas mais similares para esses filmes (Nesse projeto, são utilizados os 10 usuários mais parecidos).
- 3. Recomenda para o usuário alvo os filmes que os usuários similares já assistiram e deram notas altas.

Exemplo: 

![Tabela De Avaliações para o exemplo](/imgs/table_euclidean_example.png)

Para gerar recomendações para o usuário 1:

- 1. Verifica quais usuários assistiram filmes em comum com ele.
- 2. Como o usuário 2 não possui filmes em comum com o usuário 1, ele é ignorado.
- 3. O usuário 3 possui 2 filmes em comum com o usuário 1 (10 e 12).
- 4. É calculada a distância euclidiana com a fórmula para os usuários em comum:

![Fórmula da distância por euclidiano](/imgs/formula_euclidean_example.png)

- 5. Verifica-se quais foram os usuários com menor distância (maior similaridade).
- 6. Calcula-se um peso, que é inversamente proporcional à distância: (1.0 / (1.0 + distancia)).
- 7. Agora para cada filme não assistido, é calculada uma média ponderada, utilizando o peso e a nota dos filmes não assistidos ainda.
- 8. Essa média é uma predição da nota que o usuário alvo poderia dar para o filme.
- 9. São selecionados os top 10 filmes com maior média, sendo esses os filmes recomendados para o usuário alvo.

Média de tempo para gerar recomendações para um usuário com esse método (10 testes para os mesmos usuários aleatórios): 330.28 milisegundos.

### Correlação de Pearson

Retorno: entre -1 e 1. Quanto mais perto de 0 pior(mais desperso). Quanto mais próximo de 1 ou -1, melhor (mais concentrado).
<img src="imgs/pearson.png"><img>

Considerando "r" como o resultado da relação, tem-se: <br>
-r = |±1| → relação linear perfeita;<br>
-r = |±0,70| → relação linear forte;<br>
-r = |±0,50| → relação linear moderada;<br>
-r = |±0,30| → relação linear fraca;<br>
-r = 0 → ausência de relação linear.

Como funciona o cálculo: raiz da soma dos produtos dos desvios dividida pelo produto dos desvios padrão.
<img src="imgs/pearsonFormula.png"><img> <br>
TEMPO: Person levou em média 617,6 ms para gerar 1 recomendação

### Jaccard

A Similaridade de Jaccard é uma medida estatística que calcula a similaridade entre dois conjuntos, com base na interseção e na união desses conjuntos. Essa métrica é definida como o tamanho da interseção dividido pelo tamanho da união dos conjuntos. Em outras palavras, a Similaridade de Jaccard mede a proporção de elementos comuns entre os conjuntos em relação ao total de elementos presentes nos conjuntos.

- Formula

<div align="center" style="font-size: 24px; font-family: Arial, sans-serif;">
  <b>J(A, B) = |A ∩ B| / |A ∪ B|</b>
</div>

Onde: 
 - A∩B = elementos em comum entre A e B;
 - A∪B= todos os elementos únicos de A e B juntos.


#### Funcionamento

 1° Comparação entre usuários:
  - Compara um usuário desejado com os outros com base nos filmes assistidos em comum.
  - Avalia quais são os usuários com perfis mais similares

2° Gerar as recomendações:
 - Recomenda para o usuário alvo os filmes que os usuários similares já assistiram, mas que ele ainda não viu.
 - Prioriza filmes que aparecem com mais frequência entre os usuários similares.

#### Exemplo:

<img src="imgs/tabelajaccard.jpg" width="250">

Processamento para o Usuário 1

 - *1°: Encontra usuários com filmes em comum:*

   - Usuário 3 tem 2 filmes em comum com o Usuário 1 (filmes100 e 102).
   - Usuário 2 não tem filmes em comum e é ignorado.

- *2°:  Calcula a similaridade de Jaccard:*

  - Interseção (filmes em comum): 2 (100 e 102)
  - União (total de filmes únicos): 5 (100, 102, 103, 121, 122)
  - Similaridade = 2/5 = 0.4

- *3° Gerar as recomendações:*

  - Filmes do Usuário 3 não vistos pelo Usuário 1: {121, 212}
  - Como só tem um usuário similar (Usuário 3), ambos os filmes são recomendados com peso 1.

### Especificações do computador de testes

Os testes do algoritmo foram feitos no seguinte dispositivo:

```
Modelo: Acer Nitro ANV15-51;
CPU: 13th Gen Intel® Core™ i5-13420H × 12;
Memória: 8.0 GiB
Disco: SSD 512 GB
Sistema Operacional: Ubuntu 24.04.1 LTS 
```

### Manhattan

A distância de Manhattan é uma métrica usada para determinar a distância entre dois pontos em um caminho semelhante a uma grade. Ela mede a soma das diferenças absolutas entre as coordenadas dos pontos.
Matematicamente, a distância de Manhattan entre dois pontos em um espaço n-dimensional é a soma das diferenças absolutas de suas coordenadas cartesianas.

#### Propriedades 
A distância de Manhattan satisfaz todas as quatro condições necessárias para uma função de distância em um espaço métrico:

- 1. *Não negatividade:* A distância entre dois pontos quaisquer é sempre não negativa. d(x, y) ≥ 0 para todos os x e y.
- 2. *Identidade de indiscerníveis:* A distância entre um ponto e ele mesmo é zero, e se a distância entre dois pontos for zero, eles são o mesmo ponto. d(x, y) = 0 se e somente se x = y.
- 3. *Simetria:* A distância do ponto A ao ponto B é a mesma que a distância de B a A. d(x, y) = d(y, x) para todos os x e y.
- 4. *Desigualdade triangular:* A distância entre dois pontos é sempre menor ou igual à soma das distâncias entre esses pontos e um terceiro ponto. d(x, z) ≤ d(x, y) + d(y, z) para todos os x, y e z.

Diferentemente da distância de cosseno, que não satisfaz a desigualdade triangular, a adesão da distância de Manhattan a todas essas propriedades a torna útil em várias aplicações matemáticas e computacionais. Por exemplo:

- Em algoritmos de otimização, a desigualdade triangular pode ser usada para eliminar espaços de pesquisa com eficiência.
- Em estruturas de dados como árvores métricas, essas propriedades permitem pesquisas mais rápidas do vizinho mais próximo.
- No machine learning, os algoritmos que se baseiam em métricas de distância (como k-nearest neighbors) podem aproveitar essas propriedades para obter garantias teóricas e implementações eficientes.
  
#### Como código está implementado

O arquivo recommender_manhattan é onde o algoritmo manhattan está implementado.
- read_explore_file: Lê o arquivo datasets/explore.dat para identificar os usuários que devem receber recomendações e armazena os IDs no unordered_set<int> usersToRecommend.
- computeDistance: Calcula a distância(similiaridade) de Manhattan entre dois usuários, fazendo a soma das diferenças absolutas das notas dadas aos mesmos filmes (avaliados por ambos). Quando não há filmes em comum, retorna INFINITY (sem similaridade).
- *findSimilarUsers:*Recebe um usuário-alvo e os dados de avaliações de todos os usuários, com isso compara o usuário-alvo com todos os outros, usando computeDistance e depois ordena os usuários com a menor distância retornando os mais proximos.
- getRecommendations: Gera recomendações de filmes para o usuário-alvo, baseado nos similares. Para cada filme que o usuário-alvo ainda não viu, acumula uma média ponderada das notas dos usuários similares. Peso = 1 / (1 + distância) → mais próximo = maior peso. Retorna uma lista ordenada com os filmes recomendados e suas pontuações estimadas.
- generateRecommendations: Principal função que é responsavel por lê os usuários a serem recomendados, onde para cada usuário encontra usuários similares (findSimilarUsers), gera as recomendações(getRecommendations) e exibe elas medindo o tempo de execução de cada recomendação e calcula o tempo médio no final.


## Autores

<div>
    <strong>Arthur Lage<strong>
    <br/>
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
</div>
<br>
<div>
    <strong>Danniel Holanda<strong>
    <br/>
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
</div>
<br>
<div>
    <strong>Jade Giulia<strong>
    <br/>
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
</div>
<br>
<div>
    <strong>Luiza Magalhães<strong>
    <br/>
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
</div>
<br>
<div>
    <strong>William Leão<strong>
    <br/>
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
</div>
<br>
