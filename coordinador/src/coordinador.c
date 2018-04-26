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
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../../mySocket/src/socket.h"

void inicializar_estructuras_basicas() {

}

int main() { // ip y puerto son char* porque en la biblioteca mySocket se los necesita de ese tipo
	char* ip;
	char* puerto;
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

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config_coordinador = config_create("../config_coordinador.cfg");
	// Si no puede leer mi CFG lanza error
	if (!config_coordinador)
		log_error(logger, "No se encuentra el archivo de configuracion.");

	if (config_has_property(config_coordinador, "IP")) { // Che config_coordinador, en lo que leiste, ¿tenes el campo IP?
		ip = config_get_string_value(config_coordinador, "IP"); // Ah si, ¿lo tenes? entonces guardamelo en "ip"
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar la IP para establecer una conexion, revise su archivo de configuracion.");
	}
	// Ahh, pero si no lo tenes no te olvides de lanzar error con el logger!
	// Hace lo mismo para PUERTO y BACKLOG por favor!

	if (config_has_property(config_coordinador, "PUERTO")) {
		puerto = config_get_string_value(config_coordinador, "PUERTO");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el PUERTO para establecer una conexion, revise su archivo de configuracion.");
	}

	if (config_has_property(config_coordinador, "BACKLOG")) {
		backlog = config_get_int_value(config_coordinador, "BACKLOG");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar la cantidad maxima de conexiones que se pueden establecer, revise su archivo de configuracion.");
	}

	if (config_has_property(config_coordinador, "PACKAGESIZE")) {
		packagesize = config_get_int_value(config_coordinador, "PACKAGESIZE");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el tamaño maximo de para un paquete, revise su archivo de configuracion.");
	}

	//if (error) return -1; // Si hubo error se corta la ejecucion.
	if (!error_config)
		log_warning(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");

	log_info(logger, "Como no me reconoce el puerto lo establezco a manopla.");
	int socketDeEscucha = conectarComoServidor(logger, ip, "8000", backlog);
	int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
	recibirMensaje(logger, socketCliente, packagesize);
	finalizarSocket(socketCliente);
	finalizarSocket(socketDeEscucha);

	log_destroy(logger);
	return 0;
}

