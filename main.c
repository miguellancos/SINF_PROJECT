#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <postgresql/libpq-fe.h>
#include "ficheiros.h"


#define UNIT (255/25)

/*------------------------ STRUCTS PARA THREADS-----------------------------*/
struct thread_arguments{
    divisoes *apartamento;
    regras *regra; 
};


char *substring(char *str, int pos,int  comp){
	//Retira da string principal os 4(comp) digitos eleminando o espaco
	//devolve uma string com 4 digitos hexadecimais
	int i = 0, j = 0;
	char *subs = (char*) calloc(comp, sizeof(char));

	while (i < 20){
		if(str[pos+i] == ' '){i++; continue;}
		subs[j] = str[pos + i];
		j++;
		if (j >= comp) break;
		i++;
	}
    strcat(subs,"\0");
	return subs;
}

void* write_matrix(void * argumento){
	//Escreve no array da matriz os novos valores referentes aos ultimos dados
	//deixando os restantes valores intactos
    int i = 0, j=0,k = 0, intensity;
    char str[355],sub[15], query[200];
    divisoes *apartamento;
    PGconn *database = PQconnectdb("host='db.fe.up.pt' user='sinfa33' password='iVfwwtEp' dbname='sinfa33'");
	PGresult *values;

    apartamento =  (divisoes **) argumento;
	sprintf(str,"[");
	while (apartamento[k].nome != NULL){
        j = 0;
        i = 0;
		while (apartamento[k].atuador[j] != NULL){
			sprintf(query, "SELECT Valor FROM Atuadores WHERE Nome = '%s'",apartamento[k].atuador[j]);
			values = PQexec(database,query);
			intensity = strtol(PQgetvalue(values,0,0),NULL,10);
			
			//Define os valores de cada quadrado
			while (i < (intensity / 25)){
				strcat(str,"[255,255,255],");
				i++;
			}
			if (i < 4){
				sprintf(sub,"[%d,%d,%d],",(intensity - (25*i)) * UNIT,(intensity - (25*i)) * UNIT,(intensity - (25*i)) * UNIT);
				strcat(str,sub);
				i++;
			}
			while (i < 4){
				strcat(str,"[0,0,0],");
				i++;
			}
			j++;
		}
		k++;
	}
    str[strlen(str) - 1] = ']';
    strcat(str,"\n");
    printf("%s",str);
}

void *aplica_regras(void * arg){
    int i = 0, j = 0, conds[2];
    char query[200];
    PGresult *values;
    divisoes* apartamento;
    regras* regra;
    struct thread_arguments *arguments;
    PGconn *database = PQconnectdb("host='db.fe.up.pt' user='sinfa33' password='iVfwwtEp' dbname='sinfa33'");

    arguments = (struct thread_arguments*) arg;
    apartamento = arguments->apartamento;
    regra = arguments->regra;


    while(regra[i].divisao_id != 666){
        j = 0;
        while(regra[i].condicao[j].acoplador != 666){
            //Obtem a media dos ultimos 5 valores do sensor da regra
            sprintf(query,"SELECT ROUND(AVG(Valor),0) FROM (SELECT * FROM Leituras WHERE Sensor = '%s' ORDER BY Hora DESC LIMIT 5) AS Valores;", apartamento[regra[i].divisao_id].sensor[regra[i].condicao[j].sensor_id]);
            values = PQexec(database, query);

            
            //Verifica o resultado da condicao
             switch(regra[i].condicao[j].operando){
                case 0:   conds[0] = (strtol(PQgetvalue(values,0,0),NULL,10) < regra[i].condicao[j].valor);   break;
                case 1:   conds[0] = (strtol(PQgetvalue(values,0,0),NULL,10) == regra[i].condicao[j].valor);   break;
                case 2:   conds[0] = (strtol(PQgetvalue(values,0,0),NULL,10) > regra[i].condicao[j].valor);   break;
                default : printf("Algo de errado na condicao\n");
            }

            //Verifica as relacoes entre as condicoes
            if (regra[i].condicao[j].acoplador == 1)
                conds[1] = (conds[0] && conds[1]);
            else if (regra[i].condicao[j].acoplador == 2)
                conds[1] = (conds[0] || conds[1]);
            else
                conds[1] = conds[0];
        
            j++;
        }
        //Caso as condicoes se verifiquem executa as accoes
        j = 0;
        while (regra[i].accao[j].atuador_id != 666){
            if (conds[1]){
                if (regra[i].accao[j].variavel == 666){
                    sprintf(query,"UPDATE Atuadores SET Valor = %d, Hora = CURRENT_TIMESTAMP WHERE Nome = '%s'",regra[i].accao[j].valor,apartamento[regra[i].divisao_id].atuador[regra[i].accao[j].atuador_id]);
                }else if (regra[i].accao[j].inverso == 666){
                    sprintf(query,"SELECT ROUND(AVG(Valor),0) FROM (SELECT * FROM Leituras WHERE Sensor = '%s' ORDER BY Hora DESC LIMIT 5) AS Valores;", apartamento[regra[i].divisao_id].sensor[regra[i].condicao[j].sensor_id]);
                    values = PQexec(database, query);
                    sprintf(query,"UPDATE Atuadores SET Valor = %ld, Hora = CURRENT_TIMESTAMP WHERE Nome = '%s'",strtol(PQgetvalue(values,0,0),NULL,10),apartamento[regra[i].divisao_id].atuador[regra[i].accao[j].atuador_id]);
                }else if (regra[i].accao[j].valor == 666){
                    sprintf(query,"SELECT ROUND(AVG(Valor),0) FROM (SELECT * FROM Leituras WHERE Sensor = '%s' ORDER BY Hora DESC LIMIT 5) AS Valores;", apartamento[regra[i].divisao_id].sensor[regra[i].condicao[j].sensor_id]);
                    values = PQexec(database, query);
                    sprintf(query,"UPDATE Atuadores SET Valor = %ld, Hora = CURRENT_TIMESTAMP WHERE Nome = '%s'",(100 - strtol(PQgetvalue(values,0,0),NULL,10)),apartamento[regra[i].divisao_id].atuador[regra[i].accao[j].atuador_id]);
                }
            }
            PQexec(database,query);
            j++;
        }
        i++;
    }
    PQfinish(database);

}

void espreme_string(char* str, divisoes *apartamento){
	//Juntamente com "substring" retira todos os valores importantes
	//da string principal e converte para os valores reais
	char query[100], *aux;
    int i = 0, divisao_id = 0, valor = 0;
    PGconn *database = PQconnectdb("host='db.fe.up.pt' user='sinfa33' password='iVfwwtEp' dbname='sinfa33'");
    PGresult *resp;

	//extrai os valores da string
	divisao_id = strtol(substring(str, 15, 4), NULL, 16) - 1; 

    while(apartamento[divisao_id].sensor[i] != NULL){

	    valor = strtol(substring(str, apartamento[divisao_id].posicao[i], 4), NULL, 16);

        if (apartamento[divisao_id].posicao[i] == 36)
            valor = (0.625*15*1000*valor)/4096;
        else if (apartamento[divisao_id].posicao[i] == 42)
            valor = ((0.769*15*100*valor/4096) > 20);
        else if (apartamento[divisao_id].posicao[i] == 0){
            sprintf(query,"SELECT Valor FROM Automacao WHERE Sensor = '%s';",apartamento[divisao_id].sensor[i]);
            resp = PQexec(database, query);
            valor = (strcmp(PQgetvalue(resp,0,0),"t") == 0);
        }

        sprintf(query,"INSERT INTO Leituras(Sensor, Valor) VALUES ('%s',%d);", apartamento[divisao_id].sensor[i], valor);
        resp = PQexec(database,query);

        i++;
    }

    PQfinish(database);
}

int main()
{
    int i = 0;
    char str[75];
    regras* regra;
    divisoes* apartamento;
    struct thread_arguments arg;
    pthread_t thread[2];
    pthread_attr_t attr;

    //INICIALIZAR atributo como detached
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //INICIALIZACAO
    PGconn *database = PQconnectdb("host='db.fe.up.pt' user='sinfa33' password='iVfwwtEp' dbname='sinfa33'");
    if (PQstatus(database) != CONNECTION_OK) printf("SEM CONEXAO\n");
    apartamento = estrutura_casa(database);
    regra = leitura_regras(apartamento, database);
    PQfinish(database);

    arg.apartamento = apartamento;
    arg.regra = regra;


    while(1){
        while(fgets(str, sizeof(str), stdin) == NULL);
        if ((str[0] == '7') && (str[1] == 'E') && (str[66] == '7') && (str[67] == 'E')){}
        	espreme_string(str,apartamento);
            if (i == 5)
                pthread_create(&thread[0], &attr, aplica_regras, &arg);

            if (i = 10){
                pthread_create(&thread[1], &attr, write_matrix, &apartamento);
                i = 0;
            }
            i++;
    }

    return 0;
}