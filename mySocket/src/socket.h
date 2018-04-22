/*
 * socket.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "socket.c"
#ifndef SOCKET_H_
#define SOCKET_H_

/* Funciones de sockets que definen a los procesos como Servidor o Cliente
 * obs: backlog = cant. maxima de procesos que puede aceptar el Servidor en simultaneo
 */
void conectarComoServidor(const char* ip, const char* puerto, int backlog);
void conectarComoCliente(const char* ip, const char* puerto);

#endif /* SOCKET_H_ */
