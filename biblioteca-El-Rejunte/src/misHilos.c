/*
 * hilo.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include "misHilos.h"

void* saludar(void* arg) {
	int i = atoi((char*) arg);
	while (1) {
		printf("Hola, soy el hilo %d.\n", i);
		sleep(1);
	}
	return NULL;
}

// link de ayuda: https://www.youtube.com/watch?v=RHZ5_yWKh-0
// otro: http://www2.electron.frba.utn.edu.ar/~mdoallo/presentaciones.save.2010/clase-20100916-threads
void crear_hilo(void* (*unaFuncion) (void*), void* parametros) {
	pthread_t unHilo;

	pthread_create(&unHilo, NULL, &saludar, parametros);

	pthread_join(unHilo, NULL); // Espera a que se ejecute el hilo
}
