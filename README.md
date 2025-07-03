# Trabalho 2 - AEDS
## Recomendação de filmes

<div align="center">
    <img src="https://img.shields.io/badge/Kaggle-20BEFF?style=for-the-badge&logo=Kaggle&logoColor=white"/>
    <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>
    <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white"/>
    <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black"/>
    <img src="https://img.shields.io/badge/GIT-E44C30?style=for-the-badge&logo=git&logoColor=white"/>
    <img src="https://img.shields.io/badge/Ubuntu-E95420?style=for-the-badge&logo=ubuntu&logoColor=white"/>
</div>

## ⚠️ IMPORTANTE!

Para rodar o programa, é necessário ter os arquivos "ratings.csv" e "movies.csv" dentro de uma pasta chamada "kaggle-data", que fica localizada na raiz do projeto. Devido aos limites de armazenamento do GitHub, esses arquivos devem ser baixados separadamente pelo link abaixo:

![Base De Dados](https://www.kaggle.com/datasets/garymk/movielens-25m-dataset)

Além disso, no arquivo "datasets/explore.dat" deve estar definido, em cada linha, o id do usuário que receberá as recomendações de filmes.

As instruções mais detalhadas podem ser encontradas na seção <a href="#️-instruções-de-uso">Instruções de Uso</a>

## 📑 Sumário

- [⚠️ IMPORTANTE!](#️-importante)
- [🔭 Introdução](#-introdução)
- [📝 Metodologia](#-metodologia)
  - [Pré-processamento](#pré-processamento)
  - [Constantes](#constantes)
  - [Métodos de recomendação](#métodos-de-recomendação)
    - [Distância Euclidiana](#distância-euclidiana)
    - [Correlação de Pearson](#correlação-de-pearson)
    - [Jaccard](#jaccard)
    - [Manhattan](#manhattan)
- [⏳️ Otimizações](#️-otimizações)
- [⚙️ Fluxo do Programa](#️-fluxo-do-programa)
- [📚️ Bibliotecas](#️-bibliotecas)
- [📂 Estrutura do Projeto](#-estrutura-do-projeto)
- [🧪 Ambiente de Testes](#-ambiente-de-testes)
- [🏁 Conclusão](#-conclusão)
- [⬇️ Instruções de Uso](#️-instruções-de-uso)
- [✉️ Contato](#️-contato)

## 🔭 Introdução

Esse projeto foi desenvolvido como trabalho final da disciplina de Algoritmos e Estruturas de Dados I, ministrada por Michel Pires, do curso de Engenharia de Computação do CEFET-MG (Campus Divinópolis). O objetivo desse trabalho é gerar recomendações de filmes para usuários específicos com base numa base de dados de filmes pública, que possui mais de 25 milhões de registros, contendo identificadores para os usuários e filmes, suas avaliações e o momento em que o registro foi feito.

Nesse estudo, foi utilizada a base de dados gratuita "MovieLens 25M", que disponibiliza dados das avaliações de mais de 160 mil usuários em mais de 62 mil filmes. Esses dados foram coletados pelo serviço MovieLens entre 1995 e 2019, tendo o conjunto de dados gerado em 21 de novembro de 2019. Os arquivos usados no desenvolvimento dessa aplicação foram: 

- "ratings.csv": Arquivo que contém as avaliações dos usuários sobre determinado filme, tendo em cada linha o id do usuário, o id do filme, a nota do usuário nesse filme e o momento em que o registro foi feito (timestamp). O timestamp não foi incluido nesse estudo, já que não apresenta relevância na hora de recomendar os filmes para os usuários.

- "movies.csv": Arquivo que traz informações sobre os filmes, como: id do filme no banco de dados, nome do filme e os gêneros dele.

Dessa maneira, a partir dos dados fornecidos pelo serviço e por meio da similaridade de cossenos, que permite relacionar as avaliações para encontrar usuários semelhantes de forma rápida, foi construido esse algoritmo de recomendação de filmes. Os detalhes de implementação e execução serão detalhados ao longo dessa documentação.

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 📝 Metodologia

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

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

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>
  
#### Como código está implementado

O arquivo recommender_manhattan é onde o algoritmo manhattan está implementado.
- read_explore_file: Lê o arquivo datasets/explore.dat para identificar os usuários que devem receber recomendações e armazena os IDs no unordered_set<int> usersToRecommend.
- computeDistance: Calcula a distância(similiaridade) de Manhattan entre dois usuários, fazendo a soma das diferenças absolutas das notas dadas aos mesmos filmes (avaliados por ambos). Quando não há filmes em comum, retorna INFINITY (sem similaridade).
- *findSimilarUsers:*Recebe um usuário-alvo e os dados de avaliações de todos os usuários, com isso compara o usuário-alvo com todos os outros, usando computeDistance e depois ordena os usuários com a menor distância retornando os mais proximos.
- getRecommendations: Gera recomendações de filmes para o usuário-alvo, baseado nos similares. Para cada filme que o usuário-alvo ainda não viu, acumula uma média ponderada das notas dos usuários similares. Peso = 1 / (1 + distância) → mais próximo = maior peso. Retorna uma lista ordenada com os filmes recomendados e suas pontuações estimadas.
- generateRecommendations: Principal função que é responsavel por lê os usuários a serem recomendados, onde para cada usuário encontra usuários similares (findSimilarUsers), gera as recomendações(getRecommendations) e exibe elas medindo o tempo de execução de cada recomendação e calcula o tempo médio no final.

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

### ⏳️ Otimizações

Como o desempenho do programa foi um dos critérios mais relevantes na avaliação do trabalho, tornou-se essencial otimizar o máximo possível o programa.
Nesta seção, descreveremos os pontos chaves que levaram o tempo de execução médio do programa de quase 100 segundos para 3 segundos, uma redução de 97%, ou um ganho de performance de 32 vezes.

#### Substituição de funções do C++ pelas de C

Devido ao overhead de algumas das funcionalidades do C++, foi possível notar uma grande perda de desempenho quando utilizados: ```std::string, std::ofstream, std::ifstream, std::getline()```. Essas estruturas e funções, por implementarem muitas verificações e recursos que não foram utilizadas em nosso projeto, causaram uma redução na performance do programa, já que o processador precisa executar mais operações, que nesse caso não trazem benefícios.

Por isso, decidimos implementar as leituras e escritas de arquivo em C, usando ```FILE*, char*, fopen(), fread(), fwrite(), strtok(), strchr()```, que trouxeram uma melhora significativa no tempo de pré-processamento de dados.

#### Bufferização na leitura e escrita de arquivo

Ao invés de ler os dados um por um ou escrever cada dado separadamente no arquivo pré-processado ou de recomendações, decidimos utilizar a bufferização desses dados. Essa prática consiste em acumular os dados num buffer (uma varíavel) de um determinado tamanho, e quando esse tamanho é atingido, o programa realiza o processamento ou a escrita esses dados. Essa abordagem traz benefícios para performance, uma vez que as operações de I/O (entrada e saída) em disco custam caro para o processador, e executá-las para dezenas de milhares de dados obrigam o computador a fazer muitos acessos ao disco. Usando a bufferização, é possível reduzir bastante a quantidade de operações, diminuindo consideravelmente o tempo gasto para ler e escrever os arquivos.

#### Paralelização (Multithreading)

O uso de multithreading permite que o programa realize múltiplas tarefas ao mesmo tempo, de forma a aproveitar melhor os recursos do processador. Essa estratégia faz com que o programa separe as tarefas para núcleos diferentes do processador, tornando assim possível que essas sejam feitas simultâneamente. Isso foi de extrema importância para o cálculo das recomendações, umas vez que, sem o uso de paralelismo, o programa demorava em média 8 segundos para gerar as recomendações de 50 usuários. Porém, com a implementação dessa técnica, foi possível reduzir esse tempo para menos de 1800ms, já que partes diferentes do processador ficam responsáveis pelos cálculos, ao invés do programa executar linearmente cada um deles.

#### Recriar funções de conversão de string para int ou float

Outra medida que ajudou a reduzir bastante o tempo de execução do projeto foi criar novas funções que possibilitassem a conversão mais rápida de dados. As funçoes ```atoi(), atof()``` do C fazem a conversão de strings para inteiros e decimais, respectivamente. Entretanto, essas funções fazem muitas operações além da conversão de dados, o que desacelera o processo. Assim, recriando essa funções para fazer as conversões de forma mais simples, as quais podem ser encontradas em XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX, foi possível obter um grande aumento de desempenho, reduzindo o tempo de pré-processamento de 4500ms em média para 1000ms. Além disso, essa decisão ajudou a reduzir em cerca de 200ms o tempo de leitura e processamento do arquivo "input.dat".

## ⚙️ Fluxo do Programa

<img src="/imgs/fluxograma.jpeg">

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 📚️ Bibliotecas

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 📂 Estrutura do Projeto

Para esse projeto, utilizamos a seguinte estrutura:

```bash
movie-recommender/
│
├── datasets/                  # Pasta contendo arquivos de dados brutos
│   ├── explore.dat            # Arquivo de dados para exploração
│   └── input.dat              # Arquivo de dados de entrada
│
├── include/                   # Pasta com cabeçalhos (.hpp)
│   ├── config.hpp             # Configurações do sistema
│   ├── custom_types.hpp       # Tipos de dados customizados
│   ├── data_preprocessor.hpp  # Pré-processamento de dados
│   ├── file_handler.hpp       # Manipulação de arquivos
│   ├── input_processor.hpp    # Processamento de entrada
│   ├── movie_reader.hpp       # Leitura de dados de filmes
│   ├── output_manager.hpp     # Gerenciamento de saída
│   ├── recommender.hpp        # Lógica de recomendação
│   └── utils.hpp              # Utilitários diversos
│
├── kaggle-data/               # Dados obtidos do Kaggle
│   ├── movies.csv             # Informações de filmes
│   └── ratings.csv            # Avaliações de usuários
│
├── outcome/                   # Resultados gerados
│   └── output.txt             # Arquivo de saída com as recomendações
│
├── src/                       # Código fonte (.cpp)
│   ├── data_preprocessor.cpp  # Implementação do pré-processador
│   ├── file_handler.cpp       # Implementação do manipulador de arquivos (classe base)
│   ├── input_processor.cpp    # Implementação do processador dos dados pré-processados
│   ├── main.cpp               # Ponto de entrada do programa
│   ├── movie_reader.cpp       # Implementação do leitor de filmes
│   ├── output_manager.cpp     # Implementação do gerenciador de saída
│   ├── recommender.cpp        # Implementação do sistema de recomendação
│   └── utils.cpp              # Implementação de utilitários
│
├── .gitignore                 # Arquivo para ignorar no versionamento
├── Makefile                   # Script de compilação
├── pratica.pdf                # Instruções do trabalho
└── README.md                  # Documentação principal do projeto
```

**Estrutura e Arquivos do Sistema de Recomendação de Filmes**  

O projeto **movie-recommender** é organizado em pastas que separam os diferentes componentes do sistema, facilitando a manutenção e o desenvolvimento. A pasta **datasets/** contém arquivos essenciais para a funcionalidade principal do programa, como `explore.dat`, que descreve quais os usuários que receberão sugestões de filmes, e `input.dat`, que possui os dados filtrados e pré-processados.

Na pasta **include/**, estão os cabeçalhos (.hpp) que definem as estruturas e funções do sistema. O arquivo **config.hpp** guarda configurações globais, enquanto **custom_types.hpp** define tipos de dados personalizados para melhor organização. O pré-processamento de dados é tratado em **data_preprocessor.hpp**, e a classe base dos arquivos que usam arquivos está em **file_handler.hpp**. Já **input_processor.hpp** lida com a leitura e salvamento dos dados pré-processados, **movie_reader.hpp** concentra a leitura de informações sobre filmes, e **output_manager.hpp** controla a geração do arquivo de resultados. A lógica principal de recomendação está em **recommender.hpp**, e utilitários auxiliares são definidos em **utils.hpp**.  

Os dados externos, obtidos do Kaggle, estão em **kaggle-data/**, com **movies.csv** (metadados de filmes) e **ratings.csv** (avaliações de usuários), servindo como base para gerar as sugestões de filme. Os resultados gerados são salvos em **outcome/output.txt**, que contém as recomendações finais.  

A implementação das funcionalidades está na pasta **src/**, com arquivos como **data_preprocessor.cpp** (pré-processamento), **file_handler.cpp** (operações de arquivo), **input_processor.cpp** (processamento de dados pré-processados), e **main.cpp** (ponto de entrada). O coração do sistema, **recommender.cpp**, implementa os algoritmos de recomendação, enquanto **utils.cpp** fornece funções auxiliares.  

Arquivos como **Makefile** automatizam a compilação, **.gitignore** exclui arquivos desnecessários do versionamento, e **README.md** documenta o projeto. O **pratica.pdf** contém as instruções do trabalho, com a proposta do projeto. Essa estrutura modular garante clareza e facilidade de expansão para novas funcionalidades.

## Classes, funções e configurações

Uma descrição sobre as partes essenciais do programa:

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 🧪 Ambiente de Testes

Os testes do algoritmo foram feitos no seguinte dispositivo:

```
Modelo: Acer Nitro ANV15-51;
CPU: 13th Gen Intel® Core™ i5-13420H × 12;
Memória: 8.0 GiB
Disco: SSD 512 GB
Sistema Operacional: Ubuntu 24.04.1 LTS 
```

Além disso, foram utilizadas as ferramentas:

```
G++ (Compilador C++): gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
Makefile: GNU Make 4.3 Built for x86_64-pc-linux-gnu
```

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 🏁 Conclusão

## ⬇️ Instruções de Uso

Instruções para baixar e executar o projeto em um ambiente Ubuntu:

- 1. Dependências do projeto

```bash
# Primeiro, é recomendado atualizar os pacotes do sistema
sudo apt update -y
sudo apt upgrade -y

# Em seguida, instale as ferramentas de compilação
sudo apt install build-essential

# Se necessário, instale o make separadamente
sudo apt install make

# Caso não esteja instalado, baixe o git
sudo apt install git
```

- 2. Baixando o projeto

```bash
# Primeiro, clone o projeto para baixá-lo para a sua máquina e entre na pasta do projeto
git clone https://github.com/arthur-lage/movie-recommender.git
cd movie-recommender
```

- 3. Adicione os arquivos necessários ao projeto:

Dentro do projeto, crie uma pasta chamada "kaggle-data".

Após baixar a ![Base De Dados](https://www.kaggle.com/datasets/garymk/movielens-25m-dataset) nesse link, extraia o arquivo baixado.

Selecione os arquivos "movies.csv" e "ratings.csv" e copie-os para a pasta "kaggle-data".

- 4. Executando o projeto

```bash
# Utilize os comandos a seguir na ordem para compilar o projeto:

# Deleta os arquivos e pastas gerados na compilação
make clean

# Compila o projeto
make build

# Roda o projeto
make run


# OU


# Utilize o comando a seguir para executar os passos anteriores automaticamente:
make all
```

- 5. Resultado

O resultado das recomendações serão gerados no caminho: 

```bash
outcome/output.txt
```

## ✉️ Contato

<div align="center">
    <em>Arthur Lage - Graduando - 3º Período de Engenharia de Computação - CEFET-MG<em>
    <br/>
    <br/>
    <a href="mailto:arthurlage2006@gmail.com">
        <img src="https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://linkedin.com/in/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://github.com/arthur-lage">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
    <br/>
</div>
<br>
<div align="center">
    <em>Danniel Holanda - Graduando - 3º Período de Engenharia de Computação - CEFET-MG<em>
    <br/>
    <br/>
    <a href="mailto:dannielvh7@gmail.com">
        <img src="https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://www.linkedin.com/in/danniel-vieira-81861a334/">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://github.com/dannielvh">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
    <br/>
</div>
<br>
<div align="center">
    <em>Jade Giulia - Graduando - 3º Período de Engenharia de Computação - CEFET-MG<em>
    <br/>
    <br/>
    <a href="mailto:jadegiulia3817@gmail.com">
        <img src="https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://www.linkedin.com/in/jade-souza-352a21333">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://github.com/Jade-Souza">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
    <br/>
</div>
<br>
<div align="center">
    <em>Luiza Magalhães - Graduando - 3º Período de Engenharia de Computação - CEFET-MG<em>
    <br/>
    <br/>
    <a href="mailto:magalhaesmirandalu@gmail.com">
        <img src="https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://github.com/Lulu-blue">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
    <br/>
</div>
<br>
<div align="center">
    <em>William Leão - Graduando - 3º Período de Engenharia de Computação - CEFET-MG<em>
    <br/>
    <br/>
    <a href="mailto:wdsbleao@gmail.com">
        <img src="https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://linkedin.com/arthur-lage">
        <img src="https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white"/>
    </a>
    &nbsp;
    <a href="https://github.com/WilliamDLeao">
        <img src="https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white"/>
    </a>
    <br/>
</div>
<br>

<p align="right">(<a href="#readme-topo">voltar ao topo</a>)</p>

## 📜 Referências

[^1]: A. A. Veloso, "Classificação associativa sob demanda," Ph.D. dissertação, Departamento de Ciência da Computação, Universidade Federal de Minas Gerais, Belo Horizonte, Brasil, 2009.

[^2]: R. Cattral and F. Oppacher, *Poker Hand*, UCI Machine Learning Repository, 2007. [Online]. Available: https://doi.org/10.24432/C5KW38.

[^3]: Microsoft, "pair structure," Microsoft Learn, 2024. [Online]. Available: https://learn.microsoft.com/pt-br/cpp/standard-library/pair-structure?view=msvc-170. [Accessed: Aug. 30, 2024].

[^4]: Microsoft, "unordered_map class," Microsoft Learn, [Online]. Available: https://learn.microsoft.com/pt-br/cpp/standard-library/unordered-map-class?view=msvc-170. [Accessed: Aug. 30, 2024].

[^5]: Microsoft, "vector class," Microsoft Learn, [Online]. Available: https://learn.microsoft.com/pt-br/cpp/standard-library/vector-class?view=msvc-170. [Accessed: Aug. 30, 2024].

[^6]: L. He, Z. Gao, Q. Liu, e Z. Yang, "An Improved Grid Search Algorithm for Parameters Optimization on SVM," Applied Mechanics and Materials, vol. 644-650, pp. 2216-2221, 2014. DOI: 10.4028/www.scientific.net/AMM.644-650.2216.

[^7]: "unordered_set Class | Microsoft Learn," Microsoft, [Online]. Available: https://learn.microsoft.com/pt-br/cpp/standard-library/unordered-set-class?view=msvc-170. [Accessed: 29-Aug-2024].
