#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	Tokenizer em C++.
	Neste exmeplo, utilizamos os conceitos de classe, fila dinâmica e template.
	Criamos uma classe chamada Example, a qual representa as ações de tokenização.
	Além disso, há dentro dessa classe o conceito de fila dinâmica, onde, a variável NODE
	aparece. 
	Este exemplo, apresenta ações diversas e não contempladas no strtok. Devido a isso, 
	optamos em apresentá-la.

	Função tokenizer:
	   - Encarregada de realizar a tokenização propriamente dita. Nessa, o strtok é utilizado
	   juntamente com sua lib a <string>. O resultado dessa operação é armazenado em uma variável
	   tipo const char chamadas tokens. 
	
	Função set_delimiter:
	   - Utilizada para inicializar os separadores a serem utilizados na tokenização.
	   Essa ação é similar a variável sep utilizada nos dois exemplos contidos nesse pacote.
	
	Função get_token:
	   - Retorna um token e o remove do conjunto de tokens lidos. Nessa função estamos utilizando
	   o stod. Função que converte uma string para decimal.

	Função set_data:
	   - Função responsável por representar uma lista simplesmente encadeada, cujos nós armazenam
	   tokens. Essa estrutura é necessária caso queiramos reter os tokens após a sua leitura.
*/

int main(){

	char vet[] = "20.90, 200.119, 89.789, 45.9897";
	char sep[] = ", ' '"; //remove ',' e espaço em branco ' '
	Example<float> *toks = new Example<float>();


	toks->set_delimiter(sep);
	toks->tokenizer(vet);

	
	toks->set_data(10);
	toks->set_data(20);
	toks->set_data(30);

	//exemplo de uma fila circular sem remoção em memória
	printf("==========================================\n");
	printf("\n%f\n", toks->get_data_without_removing());	
	printf("\n%f\n", toks->get_data_without_removing());	
	printf("\n%f\n", toks->get_data_without_removing());	
	printf("\n%f\n", toks->get_data_without_removing());
	printf("\n%f\n", toks->get_data_without_removing());
	printf("\n%f\n", toks->get_data_without_removing());
	printf("==========================================\n");

	//exemplo de fila com remoção de elementos
	printf("\n==========================================\n");
	printf("\n%f\n", toks->get_data_removing());	
	printf("\n%f\n", toks->get_data_removing());	
	printf("\n%f\n", toks->get_data_removing());	
	printf("\n%f\n", toks->get_data_removing());	
	printf("==========================================\n");

	printf("\n");
	return 0;

}