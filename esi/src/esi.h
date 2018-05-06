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

char* ip_coordinador; char* ip_planificador;
char* port_coordinador; char* port_planificador;
int packagesize;
bool error_config = false;
t_log* logger;

#endif /* ESI_H_ */
