/*
 * instancia.h
 *
 *  Created on: 2 may. 2018
 *      Author: utnso
 */

#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"

t_log* logger;
char* ip;
char* port;
int packagesize;
bool error_config;
t_list* tabla_entradas;

typedef struct {
	char* clave;
	int entrada_asociada;
	int size_valor_almacenado;
} t_entrada;

int cargarConfiguracion();

#endif /* INSTANCIA_H_ */
