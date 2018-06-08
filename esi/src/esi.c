/*
 ============================================================================
 Name        : esi.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso ESI
 ============================================================================
 */

#include "esi.h"

void liberarMemoria() {
	close(socketPlanificador);
	config_destroy(configuracion);
	log_destroy(logESI);
}

int filasArchivo(char* filename) {
	FILE *in_file;
    char buffer[SIZE + 1], lastchar = '\n';
    size_t bytes;
    int lines = 0;

    if (NULL == (in_file = fopen(filename, "r"))) {
        perror(filename);
        return EXIT_FAILURE;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, in_file))) {
        lastchar = buffer[bytes - 1];
        for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
            lines++;
        }
    }
    if (lastchar != '\n') {
        lines++;  /* Count the last line even if it lacks a newline */
    }
    if (ferror(in_file)) {
        perror(filename);
        fclose(in_file);
        return EXIT_FAILURE;
    }

    fclose(in_file);
    log_trace(logESI, "Number of lines in the file is %i\n", lines);
    return(lines);
}

int main(int argc, char **argv){
	char* ipPlanificador;
	int puertoPlanificador;
	int socketPlanificador;
	ContentHeader *header;

	char* path = argv[1];

	// Leo el Archivo de Configuracion //
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	puertoPlanificador = config_get_int_value(configuracion,
			"PUERTO_PLANIFICADOR");
	ipPlanificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");

	// Inicio el log
	logESI = log_create(ARCHIVO_LOG, "ESI", true, LOG_LEVEL_TRACE);

	// Valido que haya ingresado el nombre del archivo
	if (argc < 2) {
		log_error(logESI, "No ingreso el nombre del archivo");
		log_destroy(logESI);
		exit(EXIT_FAILURE);
	}

	//Leo puertos e ips de archivo de configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	ipPlanificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");
	puertoPlanificador = config_get_int_value(configuracion, "PUERTO_PLANIFICADOR");

	// Me conecto con el planificador
	socketPlanificador = clienteConectarComponente("ESI", "planificador", puertoPlanificador, ipPlanificador);

	int maxFilas = filasArchivo(path);
	parsearScript(path, maxFilas);

	// Libero memoria
	liberarMemoria();

	return 0;
}

void parsearScript(char* path, int maxFilas) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	ContentHeader *headerPlanificador, *headerCoordinador;
	char* mensaje;
	int filasLeidas = 0;
	char* ipCoordinador;
	int puertoCoordinador;

	char* mensajeCoordinador;
	int socketCoordinador;
	char* respuestaCoordinador;
	char* nombre;

	//leo puertos e ip del coordinador
	ipCoordinador = config_get_string_value(configuracion, "IP_COORDINADOR");
	puertoCoordinador = config_get_int_value(configuracion, "PUERTO_COORDINADOR");
	nombre = config_get_string_value(configuracion, "NOMBRE");

	// Valido que el archivo exista
	if ((fp = fopen(path, "r")) == NULL) {
		log_error(logESI, "El archivo '%s' no existe o es inaccesible", path);
		liberarMemoria();
		exit(EXIT_FAILURE);
	}

	while (filasLeidas < maxFilas) {
		// Espero un mensaje del planificador
		headerPlanificador = recibirHeader(socketPlanificador);

		// Cuando el planificador me habla avanzo
		if (headerPlanificador->id == PLANIFICADOR) {
			// Loggeo que me habló el planificador
			log_trace(logESI, "Recibí una señal de avanzar del coordinador");

			if ((read = getline(&line, &len, fp)) != -1) {
				t_esi_operacion parsed = parse(line);

				if (parsed.valido) {
					switch (parsed.keyword) {
						case GET:
							mensajeCoordinador = malloc(strlen("GET ") + strlen(parsed.argumentos.GET.clave) + 1);
							strcpy(mensajeCoordinador, "GET ");
							strcat(mensajeCoordinador, parsed.argumentos.GET.clave);
							mensajeCoordinador[strlen("GET ") + strlen(parsed.argumentos.GET.clave)] = '\0';
							break;

						case SET:
							mensajeCoordinador = malloc(strlen("SET ") + strlen(parsed.argumentos.SET.clave) + 1 + strlen(parsed.argumentos.SET.valor) + 1);
							strcpy(mensajeCoordinador, "SET ");
							strcat(mensajeCoordinador, parsed.argumentos.SET.clave);
							strcat(mensajeCoordinador, " ");
							strcat(mensajeCoordinador, parsed.argumentos.SET.valor);
							mensajeCoordinador[strlen("GET ") + strlen(parsed.argumentos.SET.clave) + 1 + strlen(parsed.argumentos.SET.valor)] = '\0';
							break;

						case STORE:
							mensajeCoordinador = malloc(strlen("STORE ") + strlen(parsed.argumentos.STORE.clave) +1);
							strcpy(mensajeCoordinador, "STORE ");
							strcat(mensajeCoordinador, parsed.argumentos.STORE.clave);
							mensajeCoordinador[strlen("STORE ") + strlen(parsed.argumentos.STORE.clave)] = '\0';
							break;

						default:
							log_error(logESI, "No pude interpretar <%s>\n", line);
							liberarMemoria();
							exit(EXIT_FAILURE);
					}

					// Logueo el mensaje que le voy a mandar al coordinador
					log_trace(logESI, "Le comunico al coordinador la sentencia <%s>", mensajeCoordinador);

					// Me conecto con el coordinador
					socketCoordinador = clienteConectarComponente("ESI", "coordinador", puertoCoordinador, ipCoordinador);

					// Le mando el nombre
					enviarHeader(socketCoordinador, nombre, ESI);
					enviarMensaje(socketCoordinador, nombre);

					// Le mando la sentencia
					enviarHeader(socketCoordinador, mensajeCoordinador, ESI);
					enviarMensaje(socketCoordinador, mensajeCoordinador);
					free(mensajeCoordinador);

					// Espero una respuesta del coordinador
					headerCoordinador = recibirHeader(socketCoordinador);

					// Logueo la respuesta del coordinador
					int respuesta = headerCoordinador->id;
					if (respuesta == INSTANCIA_SENTENCIA_OK || respuesta == COORDINADOR_ESI_BLOQUEAR || respuesta == COORDINADOR_ESI_CREADO) {
						log_trace(logESI, "El coordinador me respondió que salió todo OK");
					} else {
						// TODO ¿acá debería abortar o no le importa y espera que lo aborte el planificador?
						log_error(logESI, "El coordinador me respondió que hubo un error: %s", PROTOCOLO_MENSAJE[respuesta]);
					}

					filasLeidas++;
					destruir_operacion(parsed);
				} else {
					log_error(logESI, "La linea <%s> no es valida\n", line);
					liberarMemoria();
					exit(EXIT_FAILURE);
				}
			} else {
				break;
			}
		}

	}
	fclose(fp);
	if (line)
	free(line);
}
