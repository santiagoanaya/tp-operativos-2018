/*
 ============================================================================
 Name        : planificador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Planificador
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define true 1;
#define false 0;

typedef int Bool;

typedef struct {
	char* cmd;
	char* info;
	int parametros;
} COMANDO;

COMANDO comandos[] = {
		{ "pausar","Este comando aun no se ha desarrollado.", 0},
		{ "continuar","Este comando aun no se ha desarrollado.", 0},
		{ "bloquear","Este comando aun no se ha desarrollado.", 2},
		{ "desbloquear","Este comando aun no se ha desarrollado.", 1},
		{ "listar","Este comando aun no se ha desarrollado.", 1},
		{ "kill","Este comando aun no se ha desarrollado.", 1},
		{ "status","Este comando aun no se ha desarrollado.", 1},
		{ "deadlock","Este comando aun no se ha desarrollado.", 0},
		{ "quit","Finaliza al Planificador.", 0}
};

int done;

int existeComando(char* comando) {
	register int i;
	for (i = 0; comandos[i].cmd; i++) {
		if (strcmp(comando, comandos[i].cmd) == 0) {
			return i;
		}
	}
	return -1;
}

char *leerComando(char *linea) {
	char *comando;
	int i, j;
	int largocmd = 0;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
		largocmd++;
	}
	comando = malloc(largocmd + 1);
	for (j = 0; j < largocmd; j++) {
		comando[j] = linea[j];
	}
	comando[j++] = '\0';
	return comando;
}

void obtenerParametros(char **parametros, char *linea) {
	int i, j;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
	}
	(*parametros) = malloc(strlen(linea) - i);
	i++;
	for (j = 0; i < strlen(linea); j++) {
		if (linea[i] == '\0')
			break;
		(*parametros)[j] = linea[i];
		i++;
	}
	(*parametros)[j++] = '\0';
}

void verificarParametros(char *linea, int posicion) {
	int i;
	int espacios = 0;
	char *parametros;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			espacios++;
	}
	if (comandos[posicion].parametros == espacios) {
		puts("La cantidad de parametros es correcta.");
		if (espacios == 0) {
			puts("No hay parametros para mostrar.");
		} else {
			obtenerParametros(&parametros, linea);
			printf("Los parametros ingresados son: %s\n", parametros);
			free(parametros);
		}
	} else {
		puts("La cantidad de parametros es incorrecta.");
	}
}

void ejecutarComando(char *linea) {
	char *comando = leerComando(linea);
	int posicion = existeComando(comando);
	if (posicion == -1) {
		printf("%s: El comando ingresado no existe\n", comando);
	} else {
		printf("%s: El comando ingresado existe en la posicion %d\n", comando,
				posicion);
		verificarParametros(linea, posicion);
	}
	free(comando);
}

char *recortarLinea(char *string) {
	register char *s, *t;
	for (s = string; whitespace(*s); s++)
		;
	if (*s == 0)
		return s;
	t = s + strlen(s) - 1;
	while (t > s && whitespace(*t))
		t--;
	*++t = '\0';
	return s;
}

void iniciarConsola() {
	char *linea, *aux;
	done = 0;
	while (done == 0) {
		linea = readline("user@planificador: ");
		if (!linea)
			break;
		aux = recortarLinea(linea);
		if (*aux)
			ejecutarComando(aux);
		free(linea);
	}
}

int main() {
	puts("Iniciando Planificador.");
	iniciarConsola();
	puts("El Planificador se ha finalizado correctamente.");
	return 0;
}
