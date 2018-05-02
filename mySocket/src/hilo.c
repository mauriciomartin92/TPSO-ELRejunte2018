/*
 * hilo.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include <pthread.h>
#include <stdio.h>

void* saludar(void* arg) {
	int i = *((int*) arg);
	printf("Hola, soy el hilo %d.\n", i);
	return NULL;
}

// link de ayuda: https://www.youtube.com/watch?v=RHZ5_yWKh-0
// otro: http://www2.electron.frba.utn.edu.ar/~mdoallo/presentaciones.save.2010/clase-20100916-threads
void crear_hilo(int i) {
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, &saludar, (void*) &i);
	pthread_join(unHilo, NULL); // Espera a que se ejecute el hilo
}
