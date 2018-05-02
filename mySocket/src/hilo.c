/*
 * hilo.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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
void crear_hilo(int backlog) {
	pthread_t hilo1;
	pthread_t hilo2;
	pthread_t hilo3;

	pthread_create(&hilo1, NULL, &saludar, "1");
	pthread_create(&hilo2, NULL, &saludar, "2");
	pthread_create(&hilo3, NULL, &saludar, "3");

	pthread_join(hilo1, NULL); // Espera a que se ejecute el hilo
	pthread_join(hilo2, NULL); // Espera a que se ejecute el hilo
	pthread_join(hilo3, NULL); // Espera a que se ejecute el hilo
}
