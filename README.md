# Trabalho 2 - AEDS
## Recomendação de filmes

## IMPORTANTE!

Para rodar o programa, é necessário ter o arquivo ratings.csv dentro da pasta kaggle-data. É necessário baixar esse arquivo disponível no pdf do trabalho.
Além disso, no arquivo "datasets/explore.dat" devem estar definido em cada linha, o id do usuário que receberá recomendações.

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

## Autores

