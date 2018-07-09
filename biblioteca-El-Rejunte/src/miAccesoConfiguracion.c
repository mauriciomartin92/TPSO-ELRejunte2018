/*
 * accesoConfiguracion.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include "miAccesoConfiguracion.h"

t_config* conectarAlArchivo(t_log* logger, char* path, bool* error_config) {
	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config = config_create(path);
	// Si no puede leer mi CFG lanza error
	if (!config) {
		log_error(logger, "No se encuentra el archivo de configuracion.");
		*error_config = true;
	}
	return config;
}

char* obtenerCampoString(t_log* logger, t_config* config, char* campo, bool* error_config) {
	if (config_has_property(config, campo)) { // Che config_coordinador, en lo que leiste, ¿tenes el campo que te pedi?
		return config_get_string_value(config, campo); // Ah si, ¿lo tenes? entonces devolvelo
	} else {
		log_error(logger, "No se pudo detectar %s, revise su archivo de configuracion.", campo);
		*error_config = true;
		return NULL;
	}
	// Ahh, pero si no lo tenes no te olvides de lanzar error con el logger!
}

int obtenerCampoInt(t_log* logger, t_config* config, char* campo, bool* error_config) {
	if (config_has_property(config, campo)) {
		return config_get_int_value(config, campo);
	} else {
		log_error(logger, "No se pudo detectar %s, revise su archivo de configuracion.", campo);
		*error_config = true;
		return -1;
	}
}

void finalizarConexionArchivo(t_config* config) {
	config_destroy(config);
}
