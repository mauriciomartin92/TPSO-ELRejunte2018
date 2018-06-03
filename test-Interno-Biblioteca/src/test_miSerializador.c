/*
 ============================================================================
 Name        : test_miSerializador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <parsi/parser.h>
#include "../../biblioteca-El-Rejunte/src/miSerializador.h"

int main(int argc, char* argv[]) {
	t_log* logger = log_create("esi.log", "ESI", true, LOG_LEVEL_INFO);

	FILE* fp = fopen(argv[1], "r");
	if (!fp) {
		puts("Error al abrir el archivo");
		return -1;
	}

	char * line = NULL;
	size_t len = 0;
	getline(&line, &len, fp);
	printf("%s", line);
	t_esi_operacion parsed = parse(line);

	char* paquete = empaquetarInstruccion(parsed, logger);

	t_instruccion* instruccion = desempaquetarInstruccion(paquete, logger);
	destruirPaquete(paquete);

	printf("El paquete recibido es: ");
	switch (instruccion->operacion) {
	case 1:
		printf("GET %s\n", instruccion->clave);
		break;

	case 2:
		printf("SET %s %s\n", instruccion->clave, instruccion->valor);
		break;

	case 3:
		printf("STORE %s\n", instruccion->clave);
		break;

	default:
		log_error(logger, "No comprendo la instruccion.\n");
		break;
	}

}
