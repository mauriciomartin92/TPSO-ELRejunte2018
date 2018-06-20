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
int socketCoordinador, socketPlanificador;
FILE *fp;
const uint32_t PAQUETE_OK = 1;
const uint32_t continuar_pausar = 1;

int cargarConfiguracion() {
	error_config = false;

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config = conectarAlArchivo(logger, "/home/utnso/workspace/tp-2018-1c-El-Rejunte/esi/config_esi.cfg", &error_config);

	// Obtiene los datos para conectarse al coordinador y al planificador
	ip_coordinador = obtenerCampoString(logger, config, "IP_COORDINADOR", &error_config);
	ip_planificador = obtenerCampoString(logger, config, "IP_PLANIFICADOR", &error_config);
	port_coordinador = obtenerCampoString(logger, config, "PORT_COORDINADOR", &error_config);
	port_planificador = obtenerCampoString(logger, config, "PORT_PLANIFICADOR",	&error_config);

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
	socketCoordinador = conectarComoCliente(logger, ip_coordinador, port_coordinador);
	uint32_t handshake = 1;
	send(socketCoordinador, &handshake, sizeof(uint32_t), 0);

	socketPlanificador = conectarComoCliente(logger, ip_planificador, port_planificador);
	// El planificador me asigna mi ID
	/*
	uint32_t idESI;
	recv(socketPlanificador, &idESI, sizeof(uint32_t), 0);
	*/

	while (!feof(fp)) {
		uint32_t seleccion;
		recv(socketPlanificador, &seleccion, sizeof(uint32_t), 0);

		if (seleccion == 1) { // ¿Es lo que esperaba? (1 = CONTINUAR)
			log_info(logger, "El planificador solicita una instruccion");

			// Se parsea la instruccion que se le enviara al coordiandor
			t_esi_operacion instruccion = parsearLineaScript(fp);
			log_info(logger, "La instruccion fue parseada");

			// Se empaqueta la instruccion
			char* paquete = empaquetarInstruccion(instruccion, logger);

			/*
			log_info(logger, "Envio la instruccion al planificador");
			/*
			 * Se la envio al planificador porque es el proceso encargado de
			 * administrar la claves que tienen las instrucciones de los ESI
			 */
			/*
			uint32_t tam_paquete = strlen(paquete);
			send(socketPlanificador, &tam_paquete, sizeof(uint32_t), 0); // Envio el header
			send(socketPlanificador, paquete, tam_paquete, 0); // Envio el paquete
			*/

			log_info(logger, "Envio la instruccion al coordinador");

			uint32_t tam_paquete = strlen(paquete);
			send(socketCoordinador, &tam_paquete, sizeof(uint32_t), 0); // Envio el header
			send(socketCoordinador, paquete, tam_paquete, 0); // Envio el paquete

			//Esperar respuesta coordinador.
			uint32_t respuesta_coordinador;
			recv(socketCoordinador, &respuesta_coordinador, sizeof(uint32_t), 0);

			if (respuesta_coordinador == PAQUETE_OK) {
				log_info(logger, "El coordinador informa que llego correctamente");
				log_info(logger, "Le envio el resultado al planificador");
				send(socketPlanificador, &respuesta_coordinador, sizeof(uint32_t), 0);
			} else {
				log_error(logger, "El coordinador informa que no la pudo recibir");
			}
			if (paquete) destruirPaquete(paquete);
		}
	}

	finalizar();
	return EXIT_SUCCESS;
}
