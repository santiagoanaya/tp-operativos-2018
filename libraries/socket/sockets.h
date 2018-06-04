/*
 * sockets.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_
#define DEBUG_SOCKET 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <arpa/inet.h>

int MAX_CONEX;
enum PROTOCOLO {
	// MENSAJES PARA DECIR QUIEN SOY
	ESI = 1,
	INSTANCIA = 2,
	COORDINADOR = 3,

	// MENSAJES PARA AVISAR AL PLANIFICADOR SOBRE EL GET
	COORDINADOR_ESI_BLOQUEADO = 5,
	COORDINADOR_ESI_BLOQUEAR = 6,
	COORDINADOR_ESI_CREADO = 7,
	COORDINADOR_INSTANCIA_CAIDA = 8,

	// MENSAJES PARA AVISAR AL ESI POR UN SET
	COORDINADOR_ESI_ERROR_TAMANIO_CLAVE = 9,
	COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA = 10,
	COORDINADOR_ESI_ERROR_CLAVE_DESBLOQUEADA = 11
};

//estructuras
typedef struct {
  int id;
  int largo;
} __attribute__((packed)) ContentHeader;

//prototipos
int conectarClienteA(int, char*);
int enviarInformacion(int, void*, int*);
int socketServidor(int, char*, int);
int enviarHeader(int, char*, int);
int enviarMensaje(int, char*);
void recibirMensaje(int, int, char**);
ContentHeader * recibirHeader(int);
int servidorConectarComponente(int*, char*, char*);
int clienteConectarComponente(char*, char*, int, char*);

#endif /* SOCKETS_H_ */