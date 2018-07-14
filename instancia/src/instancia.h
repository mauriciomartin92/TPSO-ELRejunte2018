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
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "../../biblioteca-El-Rejunte/src/miAccesoConfiguracion.h"
#include "../../biblioteca-El-Rejunte/src/misSockets.h"
#include "../../biblioteca-El-Rejunte/src/miSerializador.h"
#include "../../coordinador/src/coordinador.h"

typedef struct {
	char* clave;
	int entrada_asociada;
	int size_valor_almacenado;
} __attribute__((packed)) t_entrada;

t_control_configuracion cargarConfiguracion();
void establecerProtocoloReemplazo();
void crearAlmacenamiento();
void generarTablaDeEntradas();
void agregarAlDiccionario(char* key, char* val);
void almacenarValorYGenerarTabla(char* clave, char* val);
void abrirArchivoInstancia(int* fileDescriptor);
void actualizarMapaMemoria();
void dumpMemoria();
void imprimirTablaDeEntradas();
t_instruccion* recibirInstruccion(int socketCoorinador);
int validarArgumentosInstruccion(t_instruccion* instruccion); // Creo que despues se borra esta funcion
void procesar(t_instruccion* instruccion);
void operacion_SET(t_instruccion* instruccion);
void operacion_SET_reemplazo(t_entrada* entrada, char* valor);
void operacion_STORE(char* clave);
t_entrada* algoritmoDeReemplazo();
t_entrada* algoritmoCircular();
t_entrada* algoritmoLRU();
t_entrada* algoritmoBSU();
uint32_t obtenerCantidadEntradasLibres();
bool comparadorDeClaves(void* estructura);
void finalizar();

#endif /* INSTANCIA_H_ */
