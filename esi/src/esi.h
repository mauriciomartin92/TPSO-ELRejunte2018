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
#include "../../biblioteca-El-Rejunte/src/miSerializador.h"

typedef enum {
	CONFIGURACION_OK,
	CONFIGURACION_ERROR
} t_control_configuracion;

t_control_configuracion cargarConfiguracion();
t_esi_operacion parsearLineaScript(FILE* fp);
void finalizar();

#endif /* ESI_H_ */
