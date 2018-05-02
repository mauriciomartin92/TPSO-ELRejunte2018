/*
 ============================================================================
 Name        : coordinador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 * Modelo ejemplo de un servidor que espera mensajes de un proceso Cliente que se conecta a un cierto puerto.
 * Al recibir un mensaje, lo imprimira por pantalla.
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
#include "../../mySocket/src/accesoConfiguracion.h"
#include "../../mySocket/src/socket.h"
#include "../../mySocket/src/hilo.h"

int main() { // ip y puerto son char* porque en la biblioteca mySocket se los necesita de ese tipo
	char* ip;
	char* port;
	int backlog, packagesize;
	bool error_config = false;

	/*
	 * Se crea el logger, es una estructura a la cual se le da forma con la biblioca "log.h", me sirve para
	 * comunicar distintos tipos de mensajes que emite el S.O. como ser: WARNINGS, ERRORS, INFO.
	 */
	t_log* logger = log_create("coordinador.log", "Coordinador", true,
			LOG_LEVEL_INFO);

	/*
	 * Se crea en la carpeta Coordinador un archivo "config_coordinador.cfg", la idea es que utilizando la
	 * biblioteca "config.h" se maneje ese CFG con el fin de que el proceso Coordinador obtenga la información
	 * necesaria para establecer su socket. El CFG contiene los campos IP, PUERTO, BACKLOG, PACKAGESIZE.
	 */

	// Importo los datos del archivo de configuracion
	t_config* config = conectarAlArchivo(logger, "../config_coordinador.cfg",
			&error_config);

	ip = obtenerCampoString(logger, config, "IP", &error_config);
	port = obtenerCampoString(logger, config, "PORT", &error_config);
	backlog = obtenerCampoInt(logger, config, "BACKLOG", &error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);

	// Valido si hubo errores
	if (!error_config) {
		log_info(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");
	} else {
		//return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}
	/*
	 int socketDeEscucha = conectarComoServidor(logger, ip, port, backlog);
	 int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
	 recibirMensaje(logger, socketCliente, packagesize);
	 finalizarSocket(socketCliente);
	 finalizarSocket(socketDeEscucha);
	 */

	crear_hilo(backlog);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

