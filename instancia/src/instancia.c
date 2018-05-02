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
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../../mySocket/src/socket.h"
#include "../../mySocket/src/accesoConfiguracion.h"

int main() {
	char* ip;
	char* port;
	int packagesize;
	bool error_config = false;

	// Creo el logger
	t_log* logger = log_create("instancia.log", "Instancia", true,
			LOG_LEVEL_INFO);

	// Importo los datos del archivo de configuracion
	t_config* config = conectarAlArchivo(logger,
			"../config_coordinador_instancia.cfg", &error_config);

	ip = obtenerCampoString(logger, config, "IP", &error_config);
	port = obtenerCampoString(logger, config, "PORT", &error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE",
			&error_config);

	// Valido si hubo errores
	if (!error_config) {
		log_info(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");
	} else {
		//return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}

	// Me conecto con el Servidor y le mando mensajes
	int socketServidor = conectarComoCliente(logger, ip, port);
	enviarMensaje(logger, socketServidor, packagesize);
	return EXIT_SUCCESS;
}

