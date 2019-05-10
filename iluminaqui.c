#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include "iluminaqui.h"
#include "ficheiros.h"

//----------------------------- Declaracoes -------------------------------------------

time_t time_out1[5] = {0};


//-------------------------------Funcoes Auxiliares------------------------------------


char *substring(char *str, int pos,int  comp){
	//Retira da string principal os 4(comp) digitos eleminando o espaco
	//devolve uma string com 4 digitos hexadecimais
	int i = 0, j = 0;
	char *subs;

	subs = (char*)malloc(comp * sizeof(char));
	while (i < 20){
		if(str[pos+i] == ' '){i++; continue;}
		strncat(subs,&str[pos+i],1);
		j++;
		if (j >= comp) break;
		i++;
	}
	return subs;
}



//--------------------------------Funcoes Principais---------------------------------------------------

void write_matrix(divisoes *apartamento){
	//Escreve no array da matriz os novos valores referentes aos ultimos dados
	//deixando os restantes valores intactos
    int i = 0, j=0,k = 0, intensity;
    char str[355],sub[15], query[200];
    PGconn *database = PQconnectdb("host='db.fe.up.pt' user='sinfa33' password='iVfwwtEp' dbname='sinfa33'");
	PGresult *values;

	sprintf(str,"[");
	while (apartamento[k].nome != NULL){
		while (apartamento[k].atuador[j] != NULL){
	
			sprintf(query, "SELECT Valor FROM Atuadores WHERE Nome = '%s'",apartamento[k].atuador[j]);
			values = PQexec(database,query);
			intensity = strtol(PQgetvalue(database,0,0),NULL,10);
			
			//Define os valores de cada quadrado
			strcat(str,"[");
			while (i < (intensity / 25)){
				if (i < 3) sprintf(sub,"%d,",255);
				else sprintf(sub,"%d",255);
				strcat(str,sub);
				i++;
			}
			if (i < 4){
				if (i < 3) sprintf(sub,"%d,",(intensity - (25*i)) * UNIT);
				else sprintf(sub,"%d",(intensity - (25*i)) * UNIT);
				strcat(str,sub);
				i++;
			}
			while (i < 4){
				if (i < 3) sprintf(sub,"%d,",0);
				else sprintf(sub,"%d",0);
				i++;
			}
			strcat(str,"]");
			j++;
		}
		k++;
	}
    strcat(str,"]\n");
    printf("%s",str);
}





