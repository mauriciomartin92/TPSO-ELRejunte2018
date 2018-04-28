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

int main() {
	char* ip;
	char* puerto;
	int packagesize;
	bool error_config = false;

	t_log* logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL_INFO);

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config_instancia = config_create("../config_coordinador_instancia.cfg");
	// Si no puede leer mi CFG lanza error
	if (!config_instancia)
		log_error(logger, "No se encuentra el archivo de configuracion.");

	if (config_has_property(config_instancia, "IP")) { // Che config_coordinador, en lo que leiste, ¿tenes el campo IP?
		ip = config_get_string_value(config_instancia, "IP"); // Ah si, ¿lo tenes? entonces guardamelo en "ip"
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar la IP para establecer una conexion, revise su archivo de configuracion.");
	}
	// Ahh, pero si no lo tenes no te olvides de lanzar error con el logger!
	// Hace lo mismo para PUERTO y BACKLOG por favor!

	if (config_has_property(config_instancia, "PUERTO")) {
		puerto = config_get_string_value(config_instancia, "PUERTO");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el PUERTO para establecer una conexion, revise su archivo de configuracion.");
	}

	if (config_has_property(config_instancia, "PACKAGESIZE")) {
		packagesize = config_get_int_value(config_instancia, "PACKAGESIZE");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el tamaño maximo de para un paquete, revise su archivo de configuracion.");
	}

	//if (error) return -1; // Si hubo error se corta la ejecucion.
	if (!error_config)
		log_warning(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");

	int socketServidor = conectarComoCliente(logger, ip, "8000");
	enviarMensaje(logger, socketServidor, packagesize);
	return 0;
}

