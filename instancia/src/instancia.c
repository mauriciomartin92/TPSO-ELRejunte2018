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

t_log* logger;
bool error_config;
char* ip;
char* port;
int packagesize;
int socketCoordinador;
char* cant_entradas;
char* tam_entradas;
t_list* tabla_entradas;

void ejecutar(t_instruccion instruccion) {

}

void imprimirArgumentosInstruccion(t_instruccion instruccion) {
	switch (instruccion.operacion) {
	case 1:
		printf("ENTRE A GET\n");
		printf("GET %s\n", instruccion.clave);
		break;

	case 2:
		printf("ENTRE A SET\n");
		printf("SET %s %s\n", instruccion.clave, instruccion.valor);
		break;

	case 3:
		printf("ENTRE A STORE\n");
		printf("STORE %s\n", instruccion.clave);
		break;

	default:
		log_error(logger, "No comprendo la instruccion.\n");
		break;
	}
}

void recibirInstruccion(int socketCoordinador) {
	// Recibo linea de script parseada
	void* paquete = malloc(sizeof(packagesize));
	if (recv(socketCoordinador, paquete, packagesize, 0) < 0) { // MSG_WAITALL
		//Hubo error al recibir la linea parseada
		log_error(logger, "Error al recibir instruccion de script.");

		send(socketCoordinador, "error", strlen("error"), 0); // Envio respuesta al Coordinador
	} else {
		log_info(logger,
				"Recibo una instruccion de script que me envia el Coordinador.");

		t_instruccion instruccion = desempaquetarInstruccion(paquete, logger);
		destruirPaquete(paquete);

		imprimirArgumentosInstruccion(instruccion);

		ejecutar(instruccion);

		/*
		 * proceso el script asignandoselo a una instancia
		 */

		/*
		 log_info(logger,
		 "Le informo al Coordinador que el paquete llego correctamente");
		 send(socketCoordinador, "ok", strlen("ok"), 0); // Envio respuesta al Coordinador
		 */
	}
}

void abrirArchivoInstancia(int* fileDescriptor) {
	/*
	 * La syscall open() nos permite abrir un archivo para escritura/lectura
	 * con permisos de usuario para realizar dichas operaciones.
	 */
	*fileDescriptor = open("instancia.txt", O_CREAT | O_RDWR,
	S_IRUSR | S_IWUSR);

	if (*fileDescriptor < 0) {
		log_error(logger, "Error al abrir el archivo de Instancia");
		finalizar();
	}
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
		log_error(logger,
				"No se pudieron obtener todos los datos correspondientes");
		return -1;
	}
	return 1;
}

void finalizar() {
	if (socketCoordinador > 0)
		finalizarSocket(socketCoordinador);
	free(cant_entradas);
	free(tam_entradas);
	list_destroy(tabla_entradas);
	log_destroy(logger);
	exit(0);
}

int main() {
	int fd;
	char* mapa_archivo;
	struct stat sb;

	error_config = false;

	// Creo el logger
	logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0)
		finalizar(); // Si hubo error, se corta la ejecucion.

	// Me conecto con el Servidor y le mando mensajes
	socketCoordinador = conectarComoCliente(logger, ip, port);
	char* handshake = "2";
	send(socketCoordinador, handshake, strlen(handshake) + 1, 0);

	// Me preparo para recibir la cantidad y el tamaño de las entradas
	cant_entradas = malloc(sizeof(int));
	tam_entradas = malloc(sizeof(int));

	if (recv(socketCoordinador, cant_entradas, sizeof(int), 0) < 0) {
		log_error(logger, "No se pudo recibir la cantidad de entradas.");
		// EXPLOTAR
	}
	if (recv(socketCoordinador, tam_entradas, sizeof(int), 0) < 0) {
		log_error(logger, "No se pudo recibir el tamaño de las entradas.");
		// EXPLOTAR
	}

	printf("cant entradas: %d\n", atoi(cant_entradas));
	printf("tam entradas: %d\n", atoi(tam_entradas));
	// Si esta todo ok:
	log_info(logger,
			"Se recibio la cantidad y tamaño de las entradas correctamente.");

	//Creo la tabla de entradas de la instancia, que consiste en una lista.
	tabla_entradas = list_create();

	//void** storage_volatil = malloc(atoi(cant_entradas) * sizeof(atoi(tam_entradas)));

	/*
	 * ---------- mmap (lo esta haciendo Julian) ----------
	 */
	abrirArchivoInstancia(&fd);
	if (fstat(fd, &sb) < 0) {
		log_error(logger, "No se pudo obtener el tamaño de archivo");
		finalizar();
	}

	printf("Tamaño de archivo: %ld\n", sb.st_size);

	//Si el tamaño del archivo es mayor a 0, es porque existía y tiene información.
	if (sb.st_size > 0) {
		/*
		 * Con mmap() paso el archivo a un bloque de memoria de igual tamaño
		 * con dirección elegida por el SO y permisos de lectura/escritura.
		 */
		mapa_archivo = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);

		for (int i = 0; i < sb.st_size; i++) {
			printf("%c", mapa_archivo[i]);
		}
		printf("\n");
	} else {
		//El archivo fue creado y está vacío

	}
	munmap(mapa_archivo, sizeof(mapa_archivo));
	close(fd);

	recibirInstruccion(socketCoordinador);

	finalizar();
	return EXIT_SUCCESS;
}
