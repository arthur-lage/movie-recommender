# Trabalho 2 - AEDS
## Recomendação de filmes

## Convenção de estilo

### Nome de arquivos

Os nomes de arquivo devem seguir o seguinte estilo:

nome_nome.cpp
nome_nome.hpp

### Nome de classes

Os nomes de classe devem seguir o seguinte estilo:

```c++
class NomeClasse
```

### Nome de funções e variáveis

Os nomes de variáveis e funções devem seguir o seguinte estilo:

```c++
int minha_variavel;
double calcula_soma(int primeiro_numero, int segundo_numero) {}
```

## Metodologia

### rating_data.hpp/cpp

Esse arquivo define a struct MovieRating, que armazena o id e avaliação de um filme.

A função "get_formated_user_line" recebe o id do usuário e um vector de MovieRating. Ela então formata no formato:

```
usuario_id item_id1:nota1 item_id2:nota2 item_id3:nota3
```

e retorna esse valor como uma string.

### file_reader.hpp/cpp

Classe responsável por fazer a leitura de arquivos.

Essa classe possui a função "process_ratings" a qual faz uma leitura dos dados do arquivo de avaliações de filme, filtrando apenas aqueles que possuem pelo menos 50 avaliações, e também os usuários que possuem pelo menos 50 avaliações. Após a filtragem, ele cria um arquivo "datasets/input.dat" que contem os dados formatados, prontos para serem utilizados futuramente na recomendação dos filmes.

### file_writer.hpp/cpp

Classe responsável pela escrita de arquivos.

### rating_database.hpp/cpp

Uma classe responsável por armazenar os dados lidos do arquivo de avaliações.

###