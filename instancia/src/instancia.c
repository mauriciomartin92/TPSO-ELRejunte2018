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

#include "instancia.h"

int main() {
	char* ip;
	char* port;
	int packagesize;
	bool error_config = false;

	// Creo el logger
	logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL_INFO);

	// Importo los datos del archivo de configuracion
	t_config* config =
			conectarAlArchivo(logger,
					"/home/utnso/workspace/tp-2018-1c-El-Rejunte/instancia/config_instancia.cfg",
					&error_config);

	ip = obtenerCampoString(logger, config, "IP_COORDINADOR", &error_config);
	port = obtenerCampoString(logger, config, "PORT_COORDINADOR",
			&error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);

	// Valido si hubo errores
	if (error_config) {
		log_error(logger, "NO SE PUDO CONECTAR CORRECTAMENTE.");
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}

	// Me conecto con el Servidor y le mando mensajes
	int socketCoordinador = conectarComoCliente(logger, ip, port);
	char* handshake = "2";
	send(socketCoordinador, handshake, strlen(handshake) + 1, 0);

	//Creo la tabla de entradas de la instancia, que consiste en una lista.
	tabla_entradas = list_create();



	return EXIT_SUCCESS;
}

