/*
 * miSerializador.h
 *
 *  Created on: 21 may. 2018
 *      Author: utnso
 */

#include <string.h>
#include <commons/log.h>
#include <parsi/parser.h>

#ifndef SRC_MISERIALIZADOR_H_
#define SRC_MISERIALIZADOR_H_

typedef struct {
	int32_t  operacion;
	char* clave;
	char* valor;
} t_instruccion;

char* buffer;
t_instruccion instruccionMutada;

char* empaquetarInstruccion(t_esi_operacion instruccion, t_log* logger);
t_instruccion desempaquetarInstruccion(char* paqueteSerializado, t_log* logger);

#endif /* SRC_MISERIALIZADOR_H_ */
