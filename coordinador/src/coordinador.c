/*
 ============================================================================
 Name        : coordinador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */
#include "coordinador.h"

int buscarClaveEnListaDeClaves(void* structClaveVoid, void* claveVoid) {
	Clave* structClave = (Clave*) structClaveVoid;
	return strcmp(structClave->nombre, (char*) claveVoid) == 0;
}

int buscarInstanciaConClave(void* instanciaVoid, void* claveVoid) {
	Instancia* instancia = (Instancia*) instanciaVoid;
	return list_find_with_param(instancia->claves, claveVoid, buscarClaveEnListaDeClaves) != NULL;
}

void manejarInstancia(int socketInstancia, int largoMensaje) {
	int tamanioInformacionEntradas = sizeof(InformacionEntradas);
	Instancia *instanciaConectada = (Instancia*) malloc(sizeof(Instancia));
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create(ARCHIVO_CONFIGURACION);

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	// Logueo la informacion recibida
	log_trace(logCoordinador, "Se conectó la instancia de nombre: %s", nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion, "CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion, "TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia, &tamanioInformacionEntradas);

	//Logueamos el envio de informacion
	log_trace(logCoordinador, "Enviamos a la Instancia: Cant. de Entradas = %d; Tam. de Entrada = %d", entradasInstancia->cantidad, entradasInstancia->tamanio);

	// Guardo la instancia en la lista
	instanciaConectada->nombre = nombreInstancia;
	instanciaConectada->socket = socketInstancia;
	instanciaConectada->caida = 0;
	instanciaConectada->claves = list_create();

	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) instanciaConectada);
	pthread_mutex_unlock(&mutexListaInstancias);

	// Libero memoria
	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instancia) {
	close(*(int*) instancia);
}

void cerrarInstancias() {
	list_destroy_and_destroy_elements(listaInstancias, (void*) closeInstancia);
}

void loguearOperacion(char* nombre, char* mensaje) {
	pthread_mutex_lock(&mutexLog);
	FILE *f = fopen("log.txt", "a");
	if (f == NULL) {
		pthread_mutex_unlock(&mutexLog);
		// Logueamos que no se pudo loguear la operacion
		log_error(logCoordinador, "No se pudo loguear la operación");
		return;
	}
	//printf("%s , %s \n" , nombre, mensaje);
	//fprintf(f, "%s || %s\n", nombre, mensaje);
	//TODO ARREGLR FPRINTF SEGMENTATION FAULT
	fclose(f);
	pthread_mutex_unlock(&mutexLog);


	// Logueo que loguié
	log_trace(logCoordinador, "Se pudo loguear: %s || %s", nombre, mensaje);
}

int tiempoRetardoFicticio() {
	t_config* configuracion;
	int retardo;

	configuracion = config_create(ARCHIVO_CONFIGURACION);
	retardo = config_get_int_value(configuracion, "RETARDO");
	config_destroy(configuracion);

	// La funcion usleep usa microsegundos (1 seg = 1000 microseg)
	return retardo * 1000;
}

void manejarEsi(int socketEsi, int socketPlanificador, int largoMensaje) {
	char* nombre;
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader * header;

	// Recibo nombre esi
	nombre = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &nombre);

	// Recibo mensaje del esi
	header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header = recibirHeader(socketEsi);
	mensaje = malloc(header->largo + 1);
	recibirMensaje(socketEsi, header->largo, &mensaje);

	// Logueo la operación
	loguearOperacion(nombre, mensaje);

	// Logueo el mensaje
	log_trace(logCoordinador, "Recibimos el mensaje: Nombre = %s; Mensaje = %s", nombre, mensaje);

	// Retraso ficticio de la ejecucion
	int retardo = tiempoRetardoFicticio();
	log_trace(logCoordinador, "Hago un retardo de %d microsegundos", retardo);
	usleep(retardo);

	// Ejecuto
	mensajeSplitted = string_split(mensaje, " ");
	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		getClave(mensajeSplitted[1], socketPlanificador, socketEsi);
		log_trace(logCoordinador, "Se ejecuto un GET");
	} else if (strcmp(mensajeSplitted[0], "SET") == 0 || strcmp(mensajeSplitted[0], "STORE") == 0) {
		ejecutarSentencia(socketEsi, socketPlanificador, mensaje, nombre);
		log_trace(logCoordinador, "Se ejecuto un %s", mensajeSplitted[0]);

		if (strcmp(mensajeSplitted[0], "SET") == 0) free(mensajeSplitted[2]);
	} else {
		log_error(logCoordinador, "Error en el mensaje enviado por el ESI");
	}

	// Libero memoria
	free(header);
	free(nombre);
	free(mensaje);
	free(mensajeSplitted[0]);
	free(mensajeSplitted[1]);
	free(mensajeSplitted);
	close(socketEsi);
}

void manejarDesconexion(int socketInstancia, int largoMensaje) {
	char* nombreInstancia = malloc(largoMensaje + 1);
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);
	Instancia* instancia = malloc(sizeof(Instancia));

	//busco instancia
	pthread_mutex_lock(&mutexListaInstancias);
	instancia = list_find_with_param(listaInstancias, nombreInstancia, strcmpVoid);

	if (instancia == NULL) {
		log_error(logCoordinador,"Error en encontrar instancia desconectada: Instrancia: %d", instancia);
		pthread_mutex_unlock(&mutexListaInstancias);
		return;
	}

	// La marco como caída
	instancia->caida = 1;
	pthread_mutex_unlock(&mutexListaInstancias);

	// Logueo la desconexión
	log_trace(logCoordinador, "Se desconectó una instancia");
}

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*) socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch (header->id) {
		case INSTANCIA:
			manejarInstancia(socketsConectados.socketComponente, header->largo);
			log_trace(logCoordinador, "Se maneja una instancia");
			break;

		case ESI:
			manejarEsi(socketsConectados.socketComponente, socketsConectados.socketPlanificador, header->largo);
			log_trace(logCoordinador, "Se maneja un ESI");
			break;

		case INSTANCIA_COORDINADOR_DESCONECTADA:
			manejarDesconexion(socketsConectados.socketComponente, header->largo);
			log_trace(logCoordinador, "Se maneja una instancia desconectada");
			break;
	}

	free(header);
}

int correrEnHilo(SocketHilos socketsConectados) {
	pthread_t idHilo;
	SocketHilos* socketsNecesarios;
	socketsNecesarios = (SocketHilos*) malloc(sizeof(SocketHilos));
	*socketsNecesarios = socketsConectados;

	if (pthread_create(&idHilo, NULL, (void*)manejarConexion, (void*)socketsNecesarios)) {
		log_error(logCoordinador, "No se pudo crear el hilo");
		free(socketsNecesarios);
		return 0;
	}

	log_trace(logCoordinador, "Hilo asignado");
	pthread_join(idHilo, NULL);

	return 1;
}

int main() {
	puts("Iniciando Coordinador.");
	int socketEscucha, socketComponente, socketConectadoPlanificador;
	SocketHilos socketsNecesarios;
	t_config* configuracion;
	int puerto;
	int maxConexiones;
	char* ipPlanificador;

	indexInstanciaEL = 0;

	// Inicio el log
		logCoordinador = log_create(ARCHIVO_LOG, "Coordinador", LOG_PRINT, LOG_LEVEL_TRACE);

	// Leo puertos e ips de archivo de configuracion
		configuracion = config_create(ARCHIVO_CONFIGURACION);
		puerto = config_get_int_value(configuracion, "PUERTO");
		ipPlanificador = config_get_string_value(configuracion, "IP");
		maxConexiones = config_get_int_value(configuracion, "MAX_CONEX");

	// Comienzo a escuchar conexiones
		socketEscucha = socketServidor(puerto, ipPlanificador, maxConexiones);

	// Se conecta el planificador
		//socketConectadoPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");

	// Logueo la conexion
		log_trace(logCoordinador, "Se conectó el planificador: Puerto=%d; Ip Planificador=%d; Máximas conexiones=%d", puerto, ipPlanificador, maxConexiones);

	// Instancio la lista de instancias
		listaInstancias = list_create();

	// Espero conexiones de ESIs e instancias
		while ((socketComponente = servidorConectarComponente(&socketEscucha, "", ""))) {
			log_trace(logCoordinador, "Se conectó un componente");

			socketsNecesarios.socketComponente = socketComponente;
			socketsNecesarios.socketPlanificador = socketConectadoPlanificador;

			if (!correrEnHilo(socketsNecesarios)) {
				close(socketComponente);
			}
		}

	// Libero memoria
		close(socketEscucha);
		close(socketConectadoPlanificador);
		free(ipPlanificador);
		config_destroy(configuracion);
		cerrarInstancias();

	return 0;
}
