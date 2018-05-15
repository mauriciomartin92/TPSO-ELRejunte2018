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

void atendeUnESI(int socketCliente) {
	do {
		void* instruccion = malloc(sizeof(t_esi_operacion));

		// Recibo linea de script parseada
		if (recv(socketCliente, instruccion, sizeof(t_esi_operacion), 0) < 0) {
			//Hubo error al recibir la linea parseada
			log_error(logger, "Error al recibir instruccion de script");
		} else {
			log_info(logger, "Recibo un paquete del ESI");
			/*
			 * proceso el script asignandoselo a una instancia
			 */
			// send(socketCliente, RESPUESTA, TAM_RESPUESTA, 0); // Envio respuesta al ESI
		}

		free(instruccion);
	} while (1);
}

void* establecerConexion(void* socketCliente) {
	log_info(logger, "Cliente conectado");

	/* Aca se utiliza el concepto de handshake.
	 * Cada Cliente manda un identificador para avisarle al Servidor
	 * quien es el que se conecta. Esto hay que hacerlo ya que nuestro
	 * Servidor es multicliente, y a cada cliente lo atiende con un
	 * hilo distinto => para saber cada hilo que ejecutar tiene que
	 * saber con quien se esta comunicando =)
	 */

	char handshake[packagesize];
	recv(*(int*) socketCliente, (void*) handshake, packagesize, 0);
	if (atoi(handshake) == 1) {
		log_info(logger, "El cliente es ESI.");
		atendeUnESI(*(int*) socketCliente);
	} else if (atoi(handshake) == 2) {
		log_info(logger, "El cliente es una instancia.");
		//atendeUnaInstancia(*(int*) socketCliente);
	} else {
		log_error(logger, "No se pudo reconocer al cliente.");
	}

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
	t_config* config =
			conectarAlArchivo(logger,
					"/home/utnso/workspace/tp-2018-1c-El-Rejunte/coordinador/config_coordinador.cfg",
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

	while (1) { // Infinitamente escucha a la espera de que se conecte alguien
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

