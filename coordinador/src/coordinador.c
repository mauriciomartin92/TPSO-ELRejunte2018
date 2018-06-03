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

t_log* logger;
bool error_config;
char* ip;
char* port;
char* algoritmo_distribucion;
int protocolo_algoritmo_distribucion;
int backlog, packagesize, cant_entradas, tam_entradas, retardo;
t_queue* cola_instancias;
int socketDeEscucha;
int clave_tid;
bool estaAtendiendoInstancia;

t_tcb* algoritmoDeDistribucion() {
	// implementar
	// paso 1: hay que hacer un switch de la variable ya cargada: algoritmo_distribucion
	// paso 2: implementar si es EL (Equitative Load) que TCB devuelve de la tabla_instancias
	// obs: hacer el case de los demas casos pero sin implementacion
	/*switch (algoritmo_distribucion) {
	 case "EL":
	 case "LSU":
	 case "KSE":
	 }*/
	switch (protocolo_algoritmo_distribucion) {
	//case 1:

	//case 2:

	default: // Equitative Load
		return (t_tcb*) queue_pop(cola_instancias); // OJO: falta volver a encolar en algun punto
	}
}

void enviarAInstancia(char* paquete, uint32_t tam_paquete) {
	log_info(logger, "enviarAInstancia: Le envio la instruccion a la Instancia correspondiente");
	t_tcb* tcb_elegido = algoritmoDeDistribucion();
	send(tcb_elegido->socket, &tam_paquete, sizeof(uint32_t), 0);
	send(tcb_elegido->socket, paquete, tam_paquete, 0);
}

void atenderESI(int socketCliente) {
	do {
		//printf("atenderESI: La cola de instancias esta vacia? %d\n", queue_is_empty(cola_instancias));

		// Recibo linea de script parseada
		uint32_t tam_paquete;
		recv(socketCliente, &tam_paquete, sizeof(uint32_t), MSG_WAITALL); // Recibo el header
		printf("tam_paquete: %d\n", tam_paquete);
		char* paquete = malloc(tam_paquete);
		int cant_recibida = recv(socketCliente, paquete, tam_paquete, MSG_WAITALL);
		printf("cant_recibida: %d\n", cant_recibida);
		if (cant_recibida < 0) {
			sleep(retardo / 1000);
			//Hubo error al recibir la linea parseada
			log_error(logger,
					"atenderESI: Error al recibir instruccion de script");
			send(socketCliente, "error", strlen("error"), 0); // Envio respuesta al ESI
		} else {
			sleep(retardo / 1000);
			log_info(logger, "atenderESI: Recibi un paquete del ESI");

			log_info(logger,
					"atenderESI: Aguarde mientras se busca una Instancia...");
			while (queue_is_empty(cola_instancias)) {
				sleep(4);
				log_warning(logger,
						"atenderESI: No hay instancias disponibles. Reintentando...");
			}
			enviarAInstancia(paquete, tam_paquete);
			destruirPaquete(paquete);

			log_info(logger,
					"atenderESI: Le informo al ESI que el paquete llego correctamente");
			send(socketCliente, "ok", strlen("ok"), 0); // Envio respuesta al ESI
		}
	} while (1);
}

void atenderInstancia(int socketCliente) {
	t_tcb* tcb = malloc(sizeof(t_tcb));
	tcb->tid = clave_tid;
	clave_tid++;
	tcb->socket = socketCliente;
	queue_push(cola_instancias, tcb);
	log_info(logger,
			"atenderInstancia: TCB de Instancia agregado a la tabla de instancias");
	printf("atenderInstancia: La direccion del nuevo TCB es %p\n",
			queue_peek(cola_instancias));
	printf("atenderInstancia: Y la cantidad de elementos es %d\n",
			queue_size(cola_instancias));

	log_info(logger,
			"atenderInstancia: Envio a la Instancia su cantidad de entradas");
	enviarPaqueteNumerico(socketCliente, cant_entradas);

	log_info(logger,
			"atenderInstancia: Envio a la Instancia el tamaño de las entradas");
	enviarPaqueteNumerico(socketCliente, tam_entradas);
}

void* establecerConexion(void* socketCliente) {
	log_info(logger, "establecerConexion: Cliente conectado");

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
		log_info(logger, "establecerConexion: El cliente es ESI.");
		atenderESI(*(int*) socketCliente);
	} else if (atoi(handshake) == 2) {
		log_info(logger, "establecerConexion: El cliente es una Instancia.");
		estaAtendiendoInstancia = true;
		atenderInstancia(*(int*) socketCliente);
		estaAtendiendoInstancia = false;
	} else {
		log_error(logger,
				"establecerConexion: No se pudo reconocer al cliente.");
	}

	return NULL;
}

// Protocolo numerico de ALGORITMO_DISTRIBUCION
void establecerProtocoloDistribucion() {
	if (strcmp(algoritmo_distribucion, "LSU")) {
		protocolo_algoritmo_distribucion = 1;
	} else if (strcmp(algoritmo_distribucion, "KSE")) {
		protocolo_algoritmo_distribucion = 2;
	} else {
		protocolo_algoritmo_distribucion = 0; // Equitative Load
	}
}

int cargarConfiguracion() {
	error_config = false;
	clave_tid = 0; // Son unicas

	/*
	 * Se crea en la carpeta Coordinador un archivo "config_coordinador.cfg", la idea es que utilizando la
	 * biblioteca "config.h" se maneje ese CFG con el fin de que el proceso Coordinador obtenga la información
	 * necesaria para establecer su socket. El CFG contiene los campos IP, PUERTO, BACKLOG, PACKAGESIZE.
	 */

	// Importo los datos del archivo de configuracion
	t_config* config =
			conectarAlArchivo(logger,
					"/home/utnso/workspace/tp-2018-1c-El-Rejunte/coordinador/config_coordinador.cfg", &error_config);

	ip = obtenerCampoString(logger, config, "IP", &error_config);
	port = obtenerCampoString(logger, config, "PORT", &error_config);
	backlog = obtenerCampoInt(logger, config, "BACKLOG", &error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);
	algoritmo_distribucion = obtenerCampoString(logger, config, "ALGORITMO_DISTRIBUCION", &error_config);
	cant_entradas = obtenerCampoInt(logger, config, "CANT_ENTRADAS", &error_config);
	tam_entradas = obtenerCampoInt(logger, config, "TAM_ENTRADAS", &error_config);
	retardo = obtenerCampoInt(logger, config, "RETARDO", &error_config);

	establecerProtocoloDistribucion();

	// Valido si hubo errores
	if (error_config) {
		log_error(logger, "No se pudieron obtener todos los datos correspondientes");
		return -1;
	}
	return 1;
}

int main() { // ip y puerto son char* porque en la biblioteca se los necesita de ese tipo
	error_config = false;
	estaAtendiendoInstancia = false;

	/*
	 * Se crea el logger, es una estructura a la cual se le da forma con la biblioca "log.h", me sirve para
	 * comunicar distintos tipos de mensajes que emite el S.O. como ser: WARNINGS, ERRORS, INFO.
	 */
	logger = log_create("coordinador.log", "Coordinador", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0)
		return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.

	log_info(logger, "main: Se crea la tabla de intancias.");
	cola_instancias = queue_create();

	socketDeEscucha = conectarComoServidor(logger, ip, port, backlog);

	while (1) { // Infinitamente escucha a la espera de que se conecte alguien
		int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
		pthread_t unHilo; // Cada conexion la delega en un hilo
		pthread_create(&unHilo, NULL, establecerConexion, (void*) &socketCliente);
		sleep(2); // sleep para poder ver algo
	}

	finalizarSocket(socketDeEscucha);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

