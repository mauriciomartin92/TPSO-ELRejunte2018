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

t_log* logger;
bool error_config = false;
char* ip_coordinador;
char* ip_planificador;
char* port_coordinador;
char* port_planificador;
int packagesize;
int socketCoordinador, socketPlanificador;
FILE *fp;

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
		log_error(logger, "No se pudieron obtener todos los datos correspondientes");
		return -1;
	}
	return 1;
}

t_esi_operacion parsearLineaScript(FILE* fp) {
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, fp);
	printf("%s", line);
	t_esi_operacion parsed = parse(line);

	/*if (parsed.valido) {
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

void finalizar() {
	if (fp) fclose(fp);
	if (socketCoordinador > 0) finalizarSocket(socketCoordinador);
	if (socketPlanificador > 0) finalizarSocket(socketPlanificador);
	log_destroy(logger);
	exit(0);
}

int main(int argc, char* argv[]) { // Recibe por parametro el path que se guarda en arv[1]
	logger = log_create("esi.log", "ESI", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0) {
		log_error(logger, "No se pudo cargar la configuracion");
		finalizar(); // Si hubo error, se corta la ejecucion.
	}

// Abro el fichero del script
	fp = fopen(argv[1], "r");
	if (!fp) {
		log_error(logger, "Error al abrir el archivo");
		finalizar();
	}

// Me conecto como Cliente al Coordinador y al Planificador
	socketCoordinador = conectarComoCliente(logger, ip_coordinador,
			port_coordinador);
	char* handshake = "1";
	send(socketCoordinador, handshake, strlen(handshake) + 1, 0);
	socketPlanificador = conectarComoCliente(logger, ip_planificador,
			port_planificador);

	while (!feof(fp)) {
		char* seleccion = "1"; // Es lo que espero recibir
		char mensaje[packagesize]; // Es donde lo voy a recibir
		recv(socketPlanificador, (void*) mensaje, packagesize, 0); // Lo recibo

		if (strcmp(seleccion, mensaje) == 0) { // ¿Es lo que esperaba?
			log_info(logger, "El planificador solicita una instruccion");

			// Se parsea la instruccion que se le enviara al coordiandor
			t_esi_operacion instruccion = parsearLineaScript(fp);
			log_info(logger, "La instruccion fue parseada");

			// Se empaqueta la instruccion
			char* paquete = empaquetarInstruccion(instruccion, logger);

			log_info(logger, "Envio la instruccion al coordinador");
			if ((send(socketCoordinador, paquete, strlen(paquete), 0)) < 0) {
				//Hubo error al enviar la linea parseada
				log_error(logger, "Error al enviar instruccion de script");
				finalizar();
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
			if (paquete) destruirPaquete(paquete);
		}
	}

	// SE CIERRA Y LIBERA LO CORRESPONDIENTE AL ESI:

	finalizar();
	return EXIT_SUCCESS;
}
