/*
 * instancia.h
 *
 *  Created on: 2 may. 2018
 *      Author: utnso
 */

#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <parsi/parser.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"
#include "../../biblioteca-El-Rejunte/src/miSerializador.h"

typedef struct {
	char* clave;
	int entrada_asociada;
	int size_valor_almacenado;
} __attribute__((packed)) t_entrada;

int cargarConfiguracion();
void abrirArchivoInstancia(int* fileDescriptor);
void recibirInstruccion(int socketCoorinador);
void imprimirArgumentosInstruccion(t_instruccion instruccion); // Creo que despues se borra esta funcion
void ejecutar(t_instruccion instruccion);
void finalizar();

#endif /* INSTANCIA_H_ */
