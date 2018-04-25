/*
 ============================================================================
 Name        : instancia.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 * Modelo ejemplo de un Cliente que envia mensajes a un Server.
 *
 * 	No se contemplan el manejo de errores en el sistema por una cuestion didactica. Tener en cuenta esto al desarrollar.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/log.h>
#include "../../mySocket/src/socket.h"

int main() {
	t_log* logger = log_create("instancia.log", "Instancia", true,
			LOG_LEVEL_INFO);
	int socketServidor = conectarComoCliente(logger, "127.0.0.1", "8000");
	enviarMensaje(logger, socketServidor, 1024);
	return 0;
}

