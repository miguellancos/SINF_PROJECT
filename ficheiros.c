#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <postgresql/libpq-fe.h>
#include "ficheiros.h"

/*-------------------FUNCOES AUXILIARES-------------------*/
void log_code(int string_code,int fatal){
	//log o erro
	//666 - erro ao alocar memoria
	//0 - Nome da divisao nao existe
	//1 - Nome do sensor nao encontrado
	//2 - Operador condicao nao valido
	//3 - atuador nao encontrado
	//const char *str[10] = {"0 - Nome da divisao nao existe"};
	//printf("FUCKING HELL(%d)\n",error_num);
	//exit(error_num);
}


divisoes* init_divisoes(int comprimento){
	divisoes* apartamento;
	int i = 0;

	apartamento = (divisoes*) malloc(sizeof(divisoes) * (comprimento +1));
	//if (apartamento == NULL) exit_code(666);
	
	while(i <= comprimento){
		apartamento[i].nome = NULL;
		apartamento[i].sensor = NULL;
		apartamento[i].atuador = NULL;
		apartamento[i].posicao = NULL;
		i++;
	}

	return apartamento;
}


regras* init_regras(int comprimento){
	//Cria um vetor de regras de tamanho "comprimento + 1"
	//O comprimento extra serve para manter a ultima regra nula de modo
	//a determinar o fim do vetor.
	//Initializa os valores para NULL(apontadores) e 666(inteiros)
	int i = 0;
	regras* regra;

	regra =(regras*) malloc(sizeof(regras) * (comprimento + 1));
	if (regra == NULL) exit(-1);

	while (i <= comprimento){
		regra[i].divisao_id = 666;
		regra[i].condicao = NULL;
		regra[i].accao = NULL;
		i++;
	}

	return regra;
}


condicoes* init_regras_condicoes(int comprimento){
	//Cria um vetor de condicoes de tamanho "comprimento + 1"
	//O comprimento extra serve para manter a ultima regra nula de modo
	//a determinar o fim do vetor.
	//Initializa os valores para 666
	int i = 0;
	condicoes* condicao;
	
	condicao = (condicoes*) malloc(sizeof(condicoes) * (comprimento + 1));
	if (condicao == NULL) exit(-1);

	while (i <= comprimento){
		condicao[i].acoplador = 666;
		condicao[i].operando = 666;
		condicao[i].sensor_id = 666;
		condicao[i].valor = 666;
		i++;
	}

	return condicao;
}


accoes* init_regras_accoes(int comprimento){
	//Cria um vetor de accoes de tamanho "comprimento + 1"
	//O comprimento extra serve para manter a ultima regra nula de modo
	//a determinar o fim do vetor.
	//Initializa os valores para 666
	int  i = 0;
	accoes* atuador;

	atuador = (accoes*) malloc(sizeof(accoes) * (comprimento + 1));
	if (atuador == NULL) exit(-1);

	while (i <= comprimento){
		atuador[i].atuador_id = 666;
		atuador[i].variavel = 666;
		atuador[i].inverso = 666;
		atuador[i].valor = 666;
		i++;
	}

	return atuador;
}


char* corta_string(char* str, char* delim){
	int i = 0;
	char* resp = (char*) malloc(sizeof(char));
	if (str == NULL)	return NULL;
	while(i < strlen(str) && (str[i]!= delim[0]) && (str[i]!='\n')){
		resp = (char*) realloc(resp,sizeof(char)*(i+2));
		resp[i] = str[i];
		i++;
	}
	if (i < 1)	return NULL;
	resp[i] = 0; //null
	return resp;
}
/*-----------------------FUNCOES INTERMEDIARIAS----------------------*/
void processa_regra(regras* regra, char* str, divisoes* apartamento){
    int ands = 0, ors = 0, virgulas = 0, i = 0, k = 0;
    char *copy;
    char **sumo;
	
    //conta o numero de "AND"s na string
	copy = str;
    while(copy = strstr(copy, "AND")){	ands++;	copy++;	}
    
    //conta o numero de "OR"s na string
    copy = str;
    while(copy = strstr(copy, "OR")){	ors++;	copy++;	}
    
    (*regra).condicao = init_regras_condicoes(ands + ors + 1);
    
    //conta o numero de ","s na string
    copy = str;
    while(copy = strstr(copy, ",")){	virgulas++;	copy++;	}
    
    (*regra).accao = init_regras_accoes(virgulas + 1);
    
    
    //separar a string em varios componentes (espremer o sumo)
    sumo = malloc(sizeof(char*) * (2*(ands+ors)+virgulas+4));
    
    while(i < (2 + 2*(ands+ors))){
    	sumo[i] = corta_string(str," ");
    	str += strlen(sumo[i]) + 1;
    	i++;
    }	
    while(i < (3 + 2*(ands+ors) + virgulas)){
    		sumo[i] = corta_string(str,",");
    	str += strlen(sumo[i]) + 1;
    	i++;
    } 
    
    
    //Verificar se a divisao existe, guardar caso sim
    i = 0;
    while(apartamento[i].nome != NULL){
    	if (strstr(sumo[0],apartamento[i].nome) != NULL){
    		(*regra).divisao_id = i;
    		break;
    	}
    	i++;
    }
    
    //Filtrar as condicoes e guardar
    i = 0;
    while (strcmp(sumo[i],"AND") == 0 || strcmp(sumo[i],"OR") == 0 || i == 0){
    	i++;
    	(*regra).condicao[k].acoplador = (!strcmp(sumo[i-1],"AND"))*1 + (!strcmp(sumo[i-1],"OR"))*2;
    	processa_condicao(&((*regra).condicao[k]),sumo[i],apartamento[(*regra).divisao_id].sensor);
    	k++;
    	i++;
    }
    
    
    //Filtras os atuadores e guardar
    k = 0;
    while (k <= virgulas){
    	processa_atuadores(&((*regra).accao[k]),sumo[i],&apartamento[(*regra).divisao_id]);
    	k++;
    }
 
}


void processa_condicao(condicoes* condicao,char* sumo, char** sensor){
	int i = 0;
	char sinal[3] = {'<','=','>'};
	
	//Verificar se o sensor existe, guardar caso sim
	while(sensor[i] != NULL){
		if (strstr(sumo,sensor[i]) != NULL){
			condicao->sensor_id = i;
			break;
		}
		i++;
	}
	/*linha de controlo*/	if(sensor[i] == NULL) log_code(6,1); //FATAL - 6 - Regra com sensor nao valido
	
	//Verifica se o operando e valido, guarda caso sim
	if (strstr(sumo,"<") != NULL) condicao->operando = 0;
	else if (strstr(sumo,"=") != NULL) condicao->operando = 1;
	else if (strstr(sumo,">") != NULL) condicao->operando = 2;
	
	//Guarda o valor a comparar
	sumo += strlen(corta_string(sumo,&(sinal[condicao->operando]))) + 1;
	condicao->valor = strtol(sumo,NULL,10);
}


void processa_atuadores(accoes* accao,char* sumo, divisoes* apartamento){
	int i = 0;
	
	//Verificar se o atuador existe, guardar caso sim
	while ((*apartamento).atuador[i] != 0){
		if(strstr(sumo, (*apartamento).atuador[i]) != NULL){
			accao->atuador_id = i;
			break;
		}
		i++;
	}
	
	//Verificar se algum sensor e usado como variavel
	i = 0;
	while ((*apartamento).sensor[i] != 0){
		if (strstr(sumo,(*apartamento).sensor[i]) != NULL){
			accao->variavel = i;
			break;
		}
		i++;
	}
	
	//Verificar se a funcao INVERSO() existe
	accao->inverso = (strstr(sumo,"INVERSO") != NULL);
	
	//Obter o valor, caso exista
	if (accao->variavel == 666){
		sumo += strlen(corta_string(sumo,":")) + 1;
		accao->valor = strtol(sumo,NULL,10);
	}
}


/*-----------------------FUNCOES PRINCIPAIS--------------------------*/
regras* leitura_regras(divisoes* apartamento, PGconn* database){
    //Le o ficheiro das regras e processa linha a linha
    //retorna todas as regras
	char *str;
	regras* regra;
	PGresult* data;
	int i = 0, linhas = 0;

	data = PQexec(database,"SELECT Regra FROM Regras");
	linhas = PQntuples(data);
    regra = init_regras(linhas);

    for(i = 0; i < linhas; i++){
		str = PQgetvalue(data,i,0);
		processa_regra(&regra[i],str,apartamento);
	}
	
	return regra;
}

divisoes* estrutura_casa(PGconn *database){
	char str[70];
	int i = 0, j = 0, linhas = 0;
	PGresult *data;

	
	//str = (char*) malloc(sizeof(char)*1000);
	
	//Obtem as divisoes da BD
	data = PQexec(database,"SELECT Nome FROM Divisoes");
	//conta o numero de divisoes
	
	linhas = PQntuples(data);
	divisoes* apartamento = init_divisoes(linhas);

	//Guarda as divisoes
	for(i = 0; i < linhas; i++)
		apartamento[i].nome = PQgetvalue(data, i, 0);

	i = 0;	
	while(apartamento[i].nome != NULL){
		//sensores
		sprintf(str,"SELECT Nome, Posicao FROM Sensores WHERE Divisao = '%s'",apartamento[i].nome);
		data = PQexec(database, str);
		linhas = PQntuples(data);
		apartamento[i].sensor = (char**) calloc(linhas + 1, sizeof(char*));
		apartamento[i].posicao = (int*) calloc(linhas + 1, sizeof(int));

		for(j = 0; j < linhas; j++){
			apartamento[i].sensor[j] = PQgetvalue(data, j, 0);
			apartamento[i].posicao[j] = strtol(PQgetvalue(data,j,1),NULL,10);
		}

		//ATUADORES
		sprintf(str,"SELECT Nome FROM Atuadores WHERE Divisao = '%s'",apartamento[i].nome);
		data = PQexec(database, str);
		linhas = PQntuples(data);
		apartamento[i].atuador = (char**) calloc(linhas + 1, sizeof(char*));

		for(j = 0; j < linhas; j++)
			apartamento[i].atuador[j] = PQgetvalue(data, j, 0);
		
		i++;
	}

	return apartamento;
}