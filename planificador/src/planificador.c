/*
 ============================================================================
 Name        : planificador.c
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

#include "planificador.h"

void estimar() {
	// Hay que implementarlo
}

void imprimir_menu() {
	int seleccion, clave, id, recurso;

	printf("\nSeleccione una operacion (1-7):\n");
	printf("\n   1. Pausar/Continuar");
	printf("\n   2. Bloquear <clave> <id>");
	printf("\n   3. Desbloquear <clave>");
	printf("\n   4. Listar <recurso>");
	printf("\n   5. Kill <ID>");
	printf("\n   6. Status <clave>");
	printf("\n   7. Deadlock");

	printf("\n\nSeleccion: ");
	scanf("%d", &seleccion);
	printf("\n");

	switch (seleccion) {
	case 1:
		printf("Pausar/Continuar ACTIVADO");
		printf("\nGracias, se esta procesando su solicitud...");
		break;

	case 2:
		printf("Bloquear ACTIVADO");
		printf("\nIngrese clave: ");
		scanf("%d", &clave);
		printf("Ingrese ID: ");
		scanf("%d", &id);
		printf("Gracias, se esta procesando su solicitud...");
		break;

	case 3:
		printf("Desbloquear ACTIVADO");
		printf("Ingrese clave: ");
		scanf("%d", &clave);
		printf("Gracias, se esta procesando su solicitud...");
		break;

	case 4:
		printf("Listar ACTIVADO");
		printf("Ingrese recurso: ");
		scanf("%d", &recurso);
		printf("Gracias, se esta procesando su solicitud...");
		break;

	case 5:
		printf("Kill ACTIVADO");
		printf("Ingrese ID: ");
		scanf("%d", &id);
		printf("Gracias, se esta procesando su solicitud...");
		break;

	case 6:
		printf("Status ACTIVADO");
		printf("\nIngrese clave: ");
		scanf("%d", &clave);
		printf("Gracias, se esta procesando su solicitud...");
		break;

	case 7:
		printf("Deadlock ACTIVADO");
		printf("\nGracias, se esta procesando su solicitud...");
		break;
	}
	printf("\n");
}

int main() {
	error_config = false;

	/* Colas para los procesos (son listas porque se actualiza el orden de ejecucion)
	 lista_t* listos;
	 lista_t* bloqueados;
	 lista_t* terminados;
	 */

	logger = log_create("coordinador_planificador.log", "Planificador",
	true, LOG_LEVEL_INFO);

	// Importo los datos del archivo de configuracion
	t_config* config = conectarAlArchivo(logger, "../config_planificador.cfg",
			&error_config);

	ip = obtenerCampoString(logger, config, "IP", &error_config);
	port = obtenerCampoString(logger, config, "PORT", &error_config);
	backlog = obtenerCampoInt(logger, config, "BACKLOG", &error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);

	// Valido si hubo errores
	if (!error_config) {
		log_info(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");
	} else {
		log_error(logger, "NO SE PUDO CONECTAR CORRECTAMENTE.");
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}

	int socketDeEscucha = conectarComoServidor(logger, ip, port, backlog);
	int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
	log_info(logger, "Cliente conectado");
	enviarMensaje(logger, socketCliente, packagesize);
	finalizarSocket(socketCliente);
	finalizarSocket(socketDeEscucha);

	//imprimir_menu();
	return EXIT_SUCCESS;
}
