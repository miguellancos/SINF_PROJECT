#ifndef ILUMINAQUI_H_
#define ILUMINAQUI_H_


#define UNIT (255/25)


typedef struct {
	int mote_id;
	int sensor[2];
} valores;

typedef struct{
	char* nome;
	char** sensor;
	char** posicao_sensor;
	char** atuador;
}divisoes_st;

struct tm *get_time();

char *substring(char *str, int pos,int  comp);

int timer_5min(valores dados);




void write_matrix(int intensity);


#endif /* ILUMINAQUI_H_ */
