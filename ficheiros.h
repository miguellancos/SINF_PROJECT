/*
 * ficheiros.h
 *
 *  Created on: 22/03/2018
 *      Author: up201506342
 */

#ifndef FICHEIROS_H_
#define FICHEIROS_H_


/*----------------- Estruturas ----------------------*/

//um vetor -> cada posicao uma condicao
typedef struct{
	int acoplador; //[0,1,2] -> [NAO EXISTE, AND, OR]
	int operando; //[0,1,2] ->["<","=",">"]
	int sensor_id; //[0,1,2,3,4] -> divisoes.sensor[sensor_id]
	int valor;//valor a comparar
}condicoes;

typedef struct{
	int atuador_id; //[0,1,2,3,4] -> divisoes.atuador[atuador_id]
	int variavel; //[0,1,2,3,4,999] -> 999 = NAO -> divisoes.sensor[variavel]
	int inverso; //[0,1] -> [NAO,SIM]
	int valor; // valor a estabelecer [0,100]%
}accoes;

typedef struct{
	char* nome;
	char** sensor;
	int *posicao;
	char** atuador;
}divisoes;

typedef struct{
	int divisao_id; //[0,1,2,3,4] -> divisoes.nome[divisao]
	condicoes* condicao;
	accoes* accao;
}regras;


/*---------------- Funcoes Auxiliares-------------------*/

void log_code(int string_code,int fatal);

divisoes* init_divisoes(int comprimento);

regras* init_regras(int comprimento);

condicoes* init_regras_condicoes(int comprimento);

accoes* init_regras_accoes(int comprimento);

char* corta_string(char* str, char* delim);

/*-------------------Funcoes Intermediarias----------------*/

void processa_regra(regras* regra, char* str, divisoes* apartamento);

void processa_condicao(condicoes* condicao,char* sumo, char** sensor);

void processa_atuadores(accoes* accao,char* sumo, divisoes* apartamento);
/*--------------------Funcoes Principais-------------------*/

regras* leitura_regras(divisoes* apartamento, PGconn* database);

divisoes* estrutura_casa(PGconn *database);


#endif /* FICHEIROS_H_ */
