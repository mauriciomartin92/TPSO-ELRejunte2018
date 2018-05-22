/*
 * miSerializador.c
 *
 *  Created on: 21 may. 2018
 *      Author: utnso
 */

#include "miSerializador.h"

void* empaquetarInstruccion(t_esi_operacion instruccion, t_log* logger) {
	log_info(logger, "Empaqueto la instruccion");

	paquete = malloc(
			2 * sizeof(int) + sizeof(instruccion.argumentos.GET.clave));

	switch (instruccion.keyword) {
	case GET:
		log_info(logger, "GET\tclave: <%s>\n",
				instruccion.argumentos.GET.clave);

		int operacion = 1;
		memcpy(paquete, &operacion, sizeof(int));
		memcpy(paquete,
				&(strlen(instruccion.argumentos.GET.clave), sizeof(int)));
		memcpy(paquete + sizeof(int), &(instruccion.argumentos.GET.clave),
				sizeof(instruccion.argumentos.GET.clave));

		break;
	case SET:
		log_info(logger, "SET\tclave: <%s>\tvalor: <%s>\n",
				instruccion.argumentos.SET.clave,
				instruccion.argumentos.SET.valor);

		paquete = realloc(
				sizeof(int) + sizeof(instruccion.argumentos.SET.clave)
						+ sizeof(instruccion.argumentos.SET.valor));

		int operacion = 2;
		memcpy(paquete, &operacion, sizeof(int));
		memcpy(paquete + sizeof(int), &(instruccion.argumentos.SET.clave),
				sizeof(instruccion.argumentos.SET.clave));
		memcpy(paquete,
				&(strlen(instruccion.argumentos.GET.clave), sizeof(int)));
		memcpy(paquete + sizeof(int) + sizeof(instruccion.argumentos.SET.clave),
				&(instruccion.argumentos.SET.valor),
				sizeof(instruccion.argumentos.SET.valor));

		break;
	case STORE:
		log_info(logger, "STORE\tclave: <%s>\n",
				instruccion.argumentos.STORE.clave);

		int operacion = 3;
		memcpy(paquete, &operacion, sizeof(int));
		memcpy(paquete,
				&(strlen(instruccion.argumentos.GET.clave), sizeof(int)));
		memcpy(paquete + sizeof(int), &(instruccion.argumentos.STORE.clave),
				sizeof(instruccion.argumentos.STORE.clave));

		break;
	default:
		log_error(logger, "No se pudo empaquetar la instruccion\n");
		paquete = NULL;
		break;
	}

	return paquete;
}

t_instruccionDeserializada* desempaquetarInstruccion(void* paquete,
		size_t mensajeTam, t_log* logger) {
	log_info(logger, "Desempaqueto la instruccion");

	t_instruccionDeserializada* instruccionDeserializada = malloc(
			sizeof(t_instruccionDeserializada));

	memcpy(&(instruccionDeserializada->operacion), paquete, sizeof(int));
	int tam_clave = 0;
	memcpy(&tam_clave, paquete + sizeof(int), sizeof(int));
	memcpy(&(instruccionDeserializada->clave), paquete + sizeof(int),
			tam_clave);

	printf("La operacion es %d\n", instruccionDeserializada->operacion);
	printf("La clave es %s\n", instruccionDeserializada->clave);

	// Si es SET:
	if (instruccionDeserializada->operacion == 2) {
		memcpy(&(instruccionDeserializada->valor),
				paquete + (2 * sizeof(int)) + tam_clave);
		printf("La clave es %s\n", instruccionDeserializada->valor);
	}

	return instruccionDeserializada;
}
