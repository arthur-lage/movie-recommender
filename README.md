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

### rating_data.hpp

Esse arquivo define a struct MovieRating, que armazena o id e avaliação de um filme.

A função "get_formated_user_line" recebe o id do usuário e um vector de MovieRating. Ela então formata no formato:

```
usuario_id item_id1:nota1 item_id2:nota2 item_id3:nota3
```

e retorna esse valor como uma string.

###