/*
 * miSerializador.h
 *
 *  Created on: 21 may. 2018
 *      Author: utnso
 */

#include <commons/log.h>
#include <parsi/parser.h>

#ifndef SRC_MISERIALIZADOR_H_
#define SRC_MISERIALIZADOR_H_

void* paquete;

typedef struct {
	int operacion;
	char* clave;
	char* valor;
} t_instruccionDeserializada;

void* empaquetarInstruccion(t_esi_operacion instruccion, t_log* logger);
t_instruccionDeserializada* desempaquetarInstruccion(void* paqueteSerializado, t_log* logger);

#endif /* SRC_MISERIALIZADOR_H_ */
