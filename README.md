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


### Especificações do computador de testes

Os testes do algoritmo foram feitos no seguinte dispositivo:

```
Modelo: Acer Nitro ANV15-51;
CPU: 13th Gen Intel® Core™ i5-13420H × 12;
Memória: 8.0 GiB
Disco: SSD 512 GB
Sistema Operacional: Ubuntu 24.04.1 LTS 
```

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