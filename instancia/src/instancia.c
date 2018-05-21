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

void imprimirArgumentosInstruccion(t_esi_operacion* instruccion) {
	log_error(logger, "HOLA KE AC");
	printf("EL KEYWORD ES: %s", (char*) (*instruccion).keyword);
	switch ((*instruccion).keyword) {
	case GET:
		printf("GET\tclave: <%s>\n", (*instruccion).argumentos.GET.clave);
		break;
	case SET:
		printf("SET\tclave: <%s>\tvalor: <%s>\n", (*instruccion).argumentos.SET.clave,
				(*instruccion).argumentos.SET.valor);
		break;
	case STORE:
		printf("STORE\tclave: <%s>\n", (*instruccion).argumentos.STORE.clave);
		break;
	}
}

void recibirInstruccion(int socketCoordinador) {
	t_esi_operacion* instruccion = malloc(sizeof(t_esi_operacion));

	// Recibo linea de script parseada
	if (recv(socketCoordinador, instruccion, sizeof(t_esi_operacion), 0) < 0) { // MSG_WAITALL
		//Hubo error al recibir la linea parseada
		log_error(logger, "Error al recibir instruccion de script.");

		send(socketCoordinador, "error", strlen("error"), 0); // Envio respuesta al Coordinador
	} else {
		log_info(logger,
				"Recibo una instruccion de script que me envia el Coordinador.");

		imprimirArgumentosInstruccion(instruccion);

		/*
		 * proceso el script asignandoselo a una instancia
		 */

		/*
		log_info(logger,
				"Le informo al Coordinador que el paquete llego correctamente");
		send(socketCoordinador, "ok", strlen("ok"), 0); // Envio respuesta al Coordinador
		*/
	}

	free(instruccion);
}

int cargarConfiguracion() {
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
		return -1;
	}
	return 1;
}

int main() {
	error_config = false;

	// Creo el logger
	logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0)
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.

	// Me conecto con el Servidor y le mando mensajes
	int socketCoordinador = conectarComoCliente(logger, ip, port);
	char* handshake = "2";
	send(socketCoordinador, handshake, strlen(handshake) + 1, 0);

	// Me preparo para recibir la cantidad y el tamaño de las entradas
	char* cant_entradas = malloc(sizeof(int));
	char* tam_entradas = malloc(sizeof(int));

	if (recv(socketCoordinador, cant_entradas, sizeof(int), 0) < 0) {
		log_error(logger, "No se pudo recibir la cantidad de entradas.");
		// EXPLOTAR
	}
	if (recv(socketCoordinador, tam_entradas, sizeof(int), 0) < 0) {
		log_error(logger, "No se pudo recibir el tamaño de las entradas.");
		// EXPLOTAR
	}

	printf("cant entradas: %d\n", atoi(cant_entradas));
	printf("tam entradas: %d\n",  atoi(tam_entradas));
	// Si esta todo ok:
	log_info(logger,
			"Se recibio la cantidad y tamaño de las entradas correctamente.");

	//Creo la tabla de entradas de la instancia, que consiste en una lista.
	tabla_entradas = list_create();

	/*
	 * ---------- mmap (lo esta haciendo Julian) ----------
	 */

	recibirInstruccion(socketCoordinador);

	return EXIT_SUCCESS;
}

