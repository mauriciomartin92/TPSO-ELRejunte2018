/*
 * hilo.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include <pthread.h>
#include <stdio.h>

void* saludar(void* args) {
	while (1) {
		printf("Hola soy un hilo.\n");
	}
	return NULL;
}

// link de ayuda: https://www.youtube.com/watch?v=RHZ5_yWKh-0
void crear_hilo() {
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, saludar, NULL);
	pthread_join(unHilo, NULL); // Espera a que se ejecute el hilo
}
