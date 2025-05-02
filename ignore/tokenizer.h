#ifndef HEADER_TOKENIZER
#define HEADER_TOKENIZER

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace std;

template<typename T> 
class Example{
	public:
		Example();
		~Example();
		
		void tokenizer(char line[]);
		void set_delimiter(char *d);
		T get_token();

		T get_data_removing();
		T get_data_without_removing();
		void set_data(T v);
		
    
    private:
    	typedef struct NODE{
    		T value;
    		NODE *prox;
    	}NODE;

    	NODE *head, *tail, *aux;
    	const char *tokens;
    	const char *delimiter; // delimiter in "," and ' ' 
    	string::size_type sz; // alias of size_t
};


template<typename T> 
Example<T>::Example(){
	tokens    = NULL;
	delimiter = NULL;
	head      = NULL;
	tail      = NULL;
	aux		  = NULL;
}

template<typename T> 
Example<T>::~Example(){
	delete tokens;
	delete delimiter;
}

template<typename T> 
void Example<T>::set_delimiter(char *d){
	delimiter = d;
}

template<typename T> 
void Example<T>::tokenizer(char line[]){
	tokens = strtok(line, delimiter);
}

template<typename T> 
T Example<T>::get_token(){
	T v = stod(tokens, &sz);
	tokens = strtok(NULL, delimiter);
	return v;
}

template<typename T> 
void Example<T>::set_data(T v){
	NODE *tmp = new NODE;
	tmp->value = v;
	tmp->prox  = NULL;
	if(head == NULL){
		head = tmp;
		tail = tmp;
	}else{
		tail->prox = tmp;
		tail = tmp;
	}
}

template<typename T> 
T Example<T>::get_data_removing(){
	NODE *tmp;
	
	if(head != NULL){
		tmp = head;
		head = tmp->prox;
		return tmp->value;
	}
	printf("[ERRO]: NÃO HÁ DADOS !!!");
	return -1;
}

template<typename T> 
T Example<T>::get_data_without_removing(){
	
	if(aux == NULL || aux->prox == NULL){
		aux = head;
		return aux == NULL?-1:aux->value;
	}
	
	aux = aux->prox;
	return aux->value;

}


#endif