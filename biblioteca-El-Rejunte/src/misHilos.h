/*
 * hilo.h
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SRC_HILO_H_
#define SRC_HILO_H_

void crear_hilo(void* (*unaFuncion) (void*), void* parametros);

#endif /* SRC_HILO_H_ */
