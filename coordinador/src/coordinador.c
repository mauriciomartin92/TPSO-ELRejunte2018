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
t_log* logger_operaciones;
bool error_config;
char* ip;
char* port;
char* algoritmo_distribucion;
int protocolo_algoritmo_distribucion;
int backlog, packagesize, retardo;
uint32_t cant_entradas, tam_entradas;
t_queue* cola_instancias;
int socketDeEscucha;
int clave_tid;
const uint32_t PAQUETE_OK = 1;

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

int enviarAInstancia(char* paquete, uint32_t tam_paquete) {
	t_tcb* tcb_elegido = algoritmoDeDistribucion();
	send(tcb_elegido->socket, &tam_paquete, sizeof(uint32_t), 0);
	send(tcb_elegido->socket, paquete, tam_paquete, 0);

	log_info(logger, "Espero la respuesta de la Instancia");

	return 1;
}

void loguearOperacion(uint32_t id, char* paquete) {

	/*
	 * Log de Operaciones (Ejemplo)
	 * ESI 		Operación
	 * ESI 1 	SET materias:K3001 Fisica 2
	 * ESI 1 	STORE materias:K3001
	 * ESI 2 	SET materias:K3002 Economia
	 */

	char* cadena_log_operaciones = string_new();
	string_append(&cadena_log_operaciones, "ESI ");
	string_append_with_format(&cadena_log_operaciones, "%i", id);
	string_append(&cadena_log_operaciones, "	");
	string_append(&cadena_log_operaciones, paquete);
	log_info(logger_operaciones, cadena_log_operaciones);
}

void atenderESI(int socketESI) {
	do {
		//printf("atenderESI: La cola de instancias esta vacia? %d\n", queue_is_empty(cola_instancias));

		log_info(logger, "Espero un paquete del ESI");

		uint32_t tam_paquete;
		if (recv(socketESI, &tam_paquete, sizeof(uint32_t), 0) <= 0) { // Recibo el header
			log_warning(logger, "El ESI se ha desconectado");
			break;
		}
		char* paquete = malloc(tam_paquete);
		recv(socketESI, paquete, tam_paquete, 0);

		//loguearOperacion(unESI->id, paquete);

		log_info(logger, "Le informo al ESI que el paquete llego correctamente");
		send(socketESI, &PAQUETE_OK, sizeof(uint32_t), 0); // Envio respuesta al ESI

		log_info(logger, "Aguarde mientras se busca una Instancia");
		while (queue_is_empty(cola_instancias)) { // HAY QUE UTILIZAR UN SEMAFORO CONTADOR
			sleep(4); // Lo pongo para que la espera activa no sea tan densa
			log_warning(logger, "No hay instancias disponibles. Reintentando...");
		}

		log_info(logger, "Le envio la instruccion a la Instancia correspondiente");
		int resultadoInstancia = enviarAInstancia(paquete, tam_paquete);
		destruirPaquete(paquete);
		if (resultadoInstancia < 0) // Instancia desconectada
			break;
	} while (1);
}

void atenderInstancia(int socketInstancia) {
	t_tcb* tcb = malloc(sizeof(t_tcb));
	tcb->tid = clave_tid;
	clave_tid++;
	tcb->socket = socketInstancia;
	queue_push(cola_instancias, tcb);
	log_info(logger, "TCB de Instancia agregado a la tabla de instancias");
	printf("La cantidad de instancias actual es %d\n", queue_size(cola_instancias));

	log_info(logger, "Envio a la Instancia su cantidad de entradas");
	send(socketInstancia, &cant_entradas, sizeof(uint32_t), 0);

	log_info(logger, "Envio a la Instancia el tamaño de las entradas");
	send(socketInstancia, &tam_entradas, sizeof(uint32_t), 0);

	uint32_t respuesta_instancia = PAQUETE_OK;
	while (recv(socketInstancia, &respuesta_instancia, sizeof(uint32_t), 0) > 0) {
		if (respuesta_instancia == PAQUETE_OK) {
			log_info(logger, "La Instancia pudo procesar el paquete");
			queue_push(cola_instancias, tcb); // Lo vuelvo a encolar
		}
	}
	log_warning(logger, "La instancia se ha desconectado");
	//queue_pop(cola_instancias, tcb); // Habria que buscarlo en la "cola" para sacarlo, entonces mejor una lista?
	finalizarSocket(socketInstancia);
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

	uint32_t handshake;
	recv(*(int*) socketCliente, &handshake, sizeof(uint32_t), 0);
	if (handshake == 1) {
		log_info(logger, "El cliente es ESI");
		atenderESI(*(int*) socketCliente);
	} else if (handshake == 2) {
		log_info(logger, "El cliente es una Instancia");
		atenderInstancia(*(int*) socketCliente);
	} else {
		log_error(logger, "No se pudo reconocer al cliente");
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
	t_config* config = conectarAlArchivo(logger, "/home/utnso/workspace/tp-2018-1c-El-Rejunte/coordinador/config_coordinador.cfg", &error_config);

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

void finalizar() {

}

int main() { // ip y puerto son char* porque en la biblioteca se los necesita de ese tipo
	error_config = false;

	/*
	 * Se crea el logger, es una estructura a la cual se le da forma con la biblioca "log.h", me sirve para
	 * comunicar distintos tipos de mensajes que emite el S.O. como ser: WARNINGS, ERRORS, INFO.
	 */
	logger = log_create("coordinador.log", "Coordinador", true, LOG_LEVEL_INFO);
	logger_operaciones = log_create("log_operaciones.log", "Log de Operaciones", true, LOG_LEVEL_INFO);

	if (cargarConfiguracion() < 0) {
		log_error(logger, "No se pudo cargar la configuracion");
		finalizar();
	}

	log_info(logger, "Se crea la tabla de intancias");
	cola_instancias = queue_create();

	socketDeEscucha = conectarComoServidor(logger, ip, port, backlog);

	while (1) { // Infinitamente escucha a la espera de que se conecte alguien
		int socketCliente = escucharCliente(logger, socketDeEscucha, backlog);
		pthread_t unHilo; // Cada conexion la delega en un hilo
		pthread_create(&unHilo, NULL, establecerConexion, (void*) &socketCliente);
	}

	finalizarSocket(socketDeEscucha);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

