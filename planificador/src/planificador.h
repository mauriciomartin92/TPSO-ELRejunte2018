/*
 * planificador.h
 *
 *  Created on: 2 may. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

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
#include <commons/collections/queue.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"

char* ip;
char* port;
int packagesize, backlog;
char* algoritmo;
bool error_config;
t_log* logger;
// t_queue* es un puntero a una cola
t_queue* listos;
t_queue* bloqueados;
t_queue* terminados;
int pid_asignacion;

typedef struct {
	int pid; //pid_t
	int socket;
} t_pcb;

int imprimirMenu();
int cargarConfiguracion();
void* administrarHilosESI(void* socketDeEscucha);
void* procesarESI(void* socketESI);

#endif /* PLANIFICADOR_H_ */
