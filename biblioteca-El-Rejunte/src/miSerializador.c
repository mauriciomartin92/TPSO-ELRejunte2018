/*
 * miSerializador.c
 *
 *  Created on: 21 may. 2018
 *      Author: utnso
 */

#include "miSerializador.h"

void* empaquetarInstruccion(t_esi_operacion instruccion, t_log* logger) {
	log_info(logger, "Empaqueto la instruccion");

	instruccionMutada.operacion = 0;
	instruccionMutada.clave = NULL;
	instruccionMutada.valor = NULL;

	switch (instruccion.keyword) {
	case GET:
		log_info(logger, "GET\tclave: <%s>\n",
				instruccion.argumentos.GET.clave);

		instruccionMutada.operacion = 1;
		instruccionMutada.clave = malloc(
				strlen(instruccion.argumentos.GET.clave) + 1);
		strcpy(instruccionMutada.clave, instruccion.argumentos.GET.clave);

		break;
	case SET:
		log_info(logger, "SET\tclave: <%s>\tvalor: <%s>\n",
				instruccion.argumentos.SET.clave,
				instruccion.argumentos.SET.valor);

		instruccionMutada.operacion = 2;
		instruccionMutada.clave = malloc(
				strlen(instruccion.argumentos.SET.clave) + 1);
		strcpy(instruccionMutada.clave, instruccion.argumentos.SET.clave);
		instruccionMutada.clave = malloc(
				strlen(instruccion.argumentos.GET.clave) + 1);
		strcpy(instruccionMutada.valor, instruccion.argumentos.SET.valor);

		break;
	case STORE:
		log_info(logger, "STORE\tclave: <%s>\n",
				instruccion.argumentos.STORE.clave);

		instruccionMutada.operacion = 3;
		instruccionMutada.clave = malloc(
				strlen(instruccion.argumentos.STORE.clave) + 1);
		strcpy(instruccionMutada.clave, instruccion.argumentos.STORE.clave);

		break;
	default:
		log_error(logger, "No se pudo empaquetar la instruccion");
		break;
	}

	void* buffer = malloc(sizeof(instruccionMutada));
	memcpy(buffer, &instruccionMutada, sizeof(instruccionMutada));

	log_info(logger, "La instruccion fue empaquetada");
	return buffer;
}

t_instruccion desempaquetarInstruccion(void* buffer, t_log* logger) {

	log_info(logger, "Desempaqueto la instruccion");

	memcpy(&instruccionMutada, buffer, sizeof(buffer));
	printf("La operacion es %d\n", instruccionMutada.operacion);
	printf("La clave es %s\n", instruccionMutada.clave);

	// Si es SET:
	if (instruccionMutada.operacion == 2) {
		printf("La clave es %s\n", instruccionMutada.valor);
	}

	return instruccionMutada;
}
