/*
 * miSerializador.c
 *
 *  Created on: 21 may. 2018
 *      Author: utnso
 */

#include "miSerializador.h"

// Analizar la opcion de enviar un paquete del estilo char* = "1 clave valor" y la funcion strtok

char* empaquetarInstruccion(t_esi_operacion instruccion, t_log* logger) {
	log_info(logger, "Empaqueto la instruccion");
	char* buffer;

	switch (instruccion.keyword) {
	case GET:
		buffer = malloc(
				strlen("1-") + strlen(instruccion.argumentos.GET.clave));
		strcpy(buffer, "1-");
		strcpy(buffer + strlen("1-"), instruccion.argumentos.GET.clave);

		break;
	case SET:
		buffer = malloc(
				strlen("2-") + strlen(instruccion.argumentos.SET.clave)
						+ strlen("-")
						+ strlen(instruccion.argumentos.SET.valor));
		strcpy(buffer, "2-");
		strcpy(buffer + strlen("2-"), instruccion.argumentos.SET.clave);
		strcpy(buffer + strlen("2-") + strlen(instruccion.argumentos.SET.clave), "-");
		strcpy(buffer + strlen("2-") + strlen(instruccion.argumentos.SET.clave) + strlen("-"),
				instruccion.argumentos.SET.valor);

		break;
	case STORE:
		buffer = malloc(
				strlen("3-") + strlen(instruccion.argumentos.STORE.clave));
		strcpy(buffer, "3-");
		strcpy(buffer + strlen("3-"), instruccion.argumentos.STORE.clave);

		break;
	default:
		log_error(logger, "No se pudo empaquetar la instruccion");
		return NULL;
	}

	log_info(logger, "La instruccion fue empaquetada");
	printf("El paquete a enviar es: %s\n", buffer);
	return buffer;
}

t_instruccion* desempaquetarInstruccion(char* buffer, t_log* logger) {
	log_info(logger, "Desempaqueto la instruccion");
	t_instruccion* instruccionMutada = malloc(sizeof(t_instruccion));

	printf("El paquete recibido es: %s\n", buffer);

	instruccionMutada->operacion = atoi(strtok(buffer, "-"));
	instruccionMutada->clave = strtok(NULL, "-");

	if (instruccionMutada->operacion == 2) {
		instruccionMutada->valor = strtok(NULL, "-");
	}

	printf("La operacion es %d\n", instruccionMutada->operacion);
	printf("La clave es %s\n", instruccionMutada->clave);

	// Si es SET:
	if (instruccionMutada->operacion == 2) {
		printf("El valor es %s\n", instruccionMutada->valor);
	}

	return instruccionMutada;
}

void destruirPaquete(void* paquete) {
	free(paquete);
}
