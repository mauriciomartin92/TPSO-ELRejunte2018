/*
 ============================================================================
 Name        : esi.c
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
#include <parsi/parser.h>
#include "../../mySocket/src/socket.h"

void _obtenerScript(int argc, char *argv) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(argv, "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		t_esi_operacion parsed = parse(line);

		if (parsed.valido) {
			switch (parsed.keyword) {
			case GET:
				printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
				break;
			case SET:
				printf("SET\tclave: <%s>\tvalor: <%s>\n",
						parsed.argumentos.SET.clave,
						parsed.argumentos.SET.valor);
				break;
			case STORE:
				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
				break;
			default:
				fprintf(stderr, "No pude interpretar <%s>\n", line);
				exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		} else {
			fprintf(stderr, "La linea <%s> no es valida\n", line);
			exit(EXIT_FAILURE);
		}
	}

	fclose(fp);
	if (line)
		free(line);
}

int main() {
	char* ip;
	char* puerto;
	int packagesize;
	bool error_config = false;

	t_log* logger = log_create("esi.log", "ESI", true, LOG_LEVEL_INFO);

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config_esi = config_create("../config_esi.cfg");
	// Si no puede leer mi CFG lanza error
	if (!config_esi)
		log_error(logger, "No se encuentra el archivo de configuracion.");

	if (config_has_property(config_esi, "IP")) { // Che config_coordinador, en lo que leiste, ¿tenes el campo IP?
		ip = config_get_string_value(config_esi, "IP"); // Ah si, ¿lo tenes? entonces guardamelo en "ip"
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar la IP para establecer una conexion, revise su archivo de configuracion.");
	}
	// Ahh, pero si no lo tenes no te olvides de lanzar error con el logger!
	// Hace lo mismo para PUERTO y BACKLOG por favor!

	if (config_has_property(config_esi, "PUERTO")) {
		puerto = config_get_string_value(config_esi, "PUERTO");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el PUERTO para establecer una conexion, revise su archivo de configuracion.");
	}

	if (config_has_property(config_esi, "PACKAGESIZE")) {
		packagesize = config_get_int_value(config_esi, "PACKAGESIZE");
	} else {
		error_config = true;
		log_error(logger,
				"No se pudo detectar el tamaño maximo de para un paquete, revise su archivo de configuracion.");
	}

	//if (error) return -1; // Si hubo error se corta la ejecucion.
	if (!error_config)
		log_warning(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");

	int socketServidor = conectarComoCliente(logger, ip, "8000");
	//enviarMensaje(logger, socketServidor, packagesize);
	_obtenerScript(1, "../script.esi");
	return 0;
}
