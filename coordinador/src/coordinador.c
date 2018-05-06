/*
 ============================================================================
 Name        : coordinador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "coordinador.h"

void* establecerConexion(void* socketCliente) {
	log_info(logger, "Cliente conectado");
	recibirMensaje(logger, *(int*) socketCliente, packagesize);
	finalizarSocket(*(int*) socketCliente);
	return NULL;
}

int main() { // ip y puerto son char* porque en la biblioteca se los necesita de ese tipo
	error_config = false;

	/*
	 * Se crea el logger, es una estructura a la cual se le da forma con la biblioca "log.h", me sirve para
	 * comunicar distintos tipos de mensajes que emite el S.O. como ser: WARNINGS, ERRORS, INFO.
	 */
	logger = log_create("coordinador.log", "Coordinador", true, LOG_LEVEL_INFO);

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
	if (error_config) {
		log_error(logger, "NO SE PUDO CONECTAR CORRECTAMENTE.");
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}

	socketDeEscucha = conectarComoServidor(logger, ip, port, backlog);

	while (1) { // Infinitamente escucha a la espera de quien que se conecte alguien
		int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
		pthread_t unHilo; // Cada conexion la delega en un hilo
		pthread_create(&unHilo, NULL, establecerConexion,
				(void*) &socketCliente);
		sleep(2); // sleep para poder ver algo
	}

	finalizarSocket(socketDeEscucha);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

