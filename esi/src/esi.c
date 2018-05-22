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

#include "esi.h"

int cargarConfiguracion() {
	error_config = false;

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config = conectarAlArchivo(logger,
			"/home/utnso/workspace/tp-2018-1c-El-Rejunte/esi/config_esi.cfg",
			&error_config);

	// Obtiene los datos para conectarse al coordinador y al planificador
	ip_coordinador = obtenerCampoString(logger, config, "IP_COORDINADOR",
			&error_config);
	ip_planificador = obtenerCampoString(logger, config, "IP_PLANIFICADOR",
			&error_config);
	port_coordinador = obtenerCampoString(logger, config, "PORT_COORDINADOR",
			&error_config);
	port_planificador = obtenerCampoString(logger, config, "PORT_PLANIFICADOR",
			&error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);

	// Valido posibles errores
	if (error_config) {
		log_error(logger, "NO SE PUDO CONECTAR CORRECTAMENTE.");
		return -1;
	}
	return 1;
}

t_esi_operacion parsearLineaScript(FILE* fp) {
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, fp);
	//printf("%s", line);
	t_esi_operacion parsed = parse(line);

	/*if (parsed.valido) { // ESTO SOLO LO TIENE QUE HACER LA INSTANCIA PARA SABER QUE INSTRUCCION ES
		switch (parsed.keyword) {
		case GET:
			printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
			break;
		case SET:
			printf("SET\tclave: <%s>\tvalor: <%s>\n",
					parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
			break;
		case STORE:
			printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
			break;
		default:
			fprintf(stderr, "No pude interpretar <%s>\n", line);
			exit(EXIT_FAILURE);

			destruir_operacion(parsed);
		}
	} else {
		fprintf(stderr, "La linea <%s> no es valida\n", line);
		exit(EXIT_FAILURE);
	}*/

	if (line)
		free(line);

	return parsed;
}

int main(int argc, char* argv[]) { // Recibe por parametro el path que se guarda en arv[1]
	logger = log_create("esi.log", "ESI", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0)
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.

// Abro el fichero del script
	FILE *fp;
	fp = fopen(argv[1], "r");
	if (!fp) {
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

// Me conecto como Cliente al Coordinador y al Planificador
	int socketCoordinador = conectarComoCliente(logger, ip_coordinador,
			port_coordinador);
	char* handshake = "1";
	send(socketCoordinador, handshake, strlen(handshake) + 1, 0);
	int socketPlanificador = conectarComoCliente(logger, ip_planificador,
			port_planificador);

	while (!feof(fp)) {
		char* seleccion = "1"; // Es lo que espero recibir
		char mensaje[packagesize]; // Es donde lo voy a recibir
		recv(socketPlanificador, (void*) mensaje, packagesize, 0); // Lo recibo

		if (strcmp(seleccion, mensaje) == 0) { // ¿Es lo que esperaba?
			log_info(logger, "El planificador solicita una instruccion");
			t_esi_operacion instruccion = parsearLineaScript(fp); // HAY QUE MANDARLO AL COORDINADOR
			log_info(logger, "La instruccion fue parseada");

			void* paquete = empaquetarInstruccion(instruccion, logger);

			log_info(logger, "Envio la instruccion al coordinador");
			if ((send(socketCoordinador, paquete, sizeof(paquete),
					0)) < 0) {
				//Hubo error al enviar la linea parseada
				log_error(logger, "Error al enviar instruccion de script");
				exit(EXIT_FAILURE);
			} else {
				//Esperar respuesta coordinador.
				char respuestaCoordinador[packagesize];
				recv(socketCoordinador, (void*) respuestaCoordinador,
						packagesize, 0);

				if (strcmp(respuestaCoordinador, "ok") == 0) {
					log_info(logger,
							"El coordinador informa que llego correctamente");
					log_info(logger, "Le envio el resultado al planificador");
					//send(socketPlanificador, respuestaCoordinador, strlen(respuestaCoordinador) + 1, 0);
				} else {
					log_error(logger,
							"El coordinador informa que no la pudo recibir");
				}
			}
		}
	}
	fclose(fp);
	return EXIT_SUCCESS;
}
