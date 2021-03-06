/*
 * algoritmoDeDistribucion.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_
#define ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_

/*
 ============================================================================
 Name        : algoritmosDistribucion.h
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Algoritmos de distribucion para el coordinador
 ============================================================================
 */


#include <commonsNuestras/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <generales/generales.h>

int indexInstanciaEL;

Instancia* algoritmoDistribucionEL(t_list*);
Instancia* algoritmoDistribucionLSU(t_list*);
bool mayorEspacioLibre(void*, void*);
Instancia* algoritmoDistribucionKE(t_list*, char* nombreClave);

#endif /* ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_ */
