/*
 * esi.h
 *
 *  Created on: 2 may. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <parsi/parser.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"

t_log* logger;
t_config* config;
bool error_config = false;
char* ip_coordinador; char* ip_planificador;
char* port_coordinador; char* port_planificador;
int packagesize;

void cargar_configuracion();
t_esi_operacion parsearLineaScript(FILE* fp);

#endif /* ESI_H_ */
