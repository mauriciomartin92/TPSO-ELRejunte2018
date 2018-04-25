/*
 * socket.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "socket.c"
#include <commons/log.h>
#ifndef SOCKET_H_
#define SOCKET_H_

/*
 * Funciones de sockets que definen a los procesos como Servidor o Cliente
 * obs: backlog = cant. maxima de procesos que puede aceptar el Servidor en simultaneo
 */
int conectarComoServidor(t_log* logger, const char* ip, const char* puerto, int backlog);
int escucharCliente(t_log* logger, int listenningSocket, int backlog);
void recibirMensaje(t_log* logger, int socketCliente, int packagesize);

int conectarComoCliente(t_log* logger, const char* ip, const char* puerto);
void enviarMensaje(t_log* logger, int socketServidor, int packagesize);

int finalizarSocket(int socket);

#endif /* SOCKET_H_ */
