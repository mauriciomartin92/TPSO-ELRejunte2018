/*
 * coordinador.h
 *
 *  Created on: 2 may. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <parsi/parser.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"

t_log* logger;
char* ip;
char* port;
char* algoritmo_distribucion;
int backlog, packagesize, cant_entradas, tam_entradas, retardo;
t_queue* cola_instancias;
int socketDeEscucha;
bool error_config;
int clave_tid;
bool estaAtendiendoInstancia;


typedef struct {
	int tid;
	int socket;
	t_list* esis_asignados;
} t_tcb;

int cargarConfiguracion();
void* establecerConexion(void* parametros);
void atenderESI(int socketCliente);
void atenderInstancia(int socketCliente);
void enviarAInstancia(t_esi_operacion* instruccion);
t_tcb* algoritmoDeDistribucion();

#endif /* COORDINADOR_H_ */
