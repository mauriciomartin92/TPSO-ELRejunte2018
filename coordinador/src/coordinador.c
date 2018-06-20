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
int retardo;
uint32_t cant_entradas, tam_entradas;
t_queue* cola_instancias;
int socketDeEscucha;
int clave_id;
const uint32_t PAQUETE_OK = 1;

/*
(t_instancia*) algoritmoLSU(cola_istancias, instancia, clave){
	analizar tamaño de entradas();
	analizar tamanño de instancias(); //tamaño = cantidad de entradas libres
	if(hay entradas libres){
		if(tamEntLibre == tamLoQQuieroGuardar) // si lo quiero guardar es atómico
			asignar clave en ésta entrada();
		else if (tamEntLibre < tamLoQQuieroGuardar) // si lo que quiero guardar ocupa más de una entrada
			buscar espacio continuo() //dos entradas libres contiguas
			si hay
				asignar clave en estas entradas()
		   	si no hay
				compactar o posiblemente seguir buscando
	}
}
(t_instancia*) algoritmoKE(cola_instancias, instancia, char clave){
	inicial = getChar("clave"); // tomar primer caracter clave EN MINUSCULA, ésto
	inicialEnMinuscula = tolower(inicial) // convierte un tipo de dato caracter a minuscula (A-Z a a-z).
	verificar donde guardar(inicialEnMinuscula == inicialInstancia) // inicial debera ser un numero, ejemplo "a" es 97
	if(está la instancia con la misma inicial){
		guardar en esa instancia
	} else {
		ACA NO SE SABE QUE HACE
	}
}
 */

t_instancia* algoritmoDeDistribucion() {
	// implementar
	// paso 1: hay que hacer un switch de la variable ya cargada: algoritmo_distribucion
	// paso 2: implementar si es EL (Equitative Load) que TCB devuelve de la tabla_instancias
	// obs: hacer el case de los demas casos pero sin implementacion

	switch (protocolo_algoritmo_distribucion) {
	case 1: // LSU
		//return algoritmoLSU();

	case 2: // KE
		//return algoritmoKE();

	default: // Equitative Load
		return (t_instancia*) queue_pop(cola_instancias); // OJO: falta volver a encolar en algun punto
	}
}

int enviarAInstancia(char* paquete, uint32_t tam_paquete) {
	t_instancia* instancia_elegida = algoritmoDeDistribucion();
	send(instancia_elegida->socket, &tam_paquete, sizeof(uint32_t), 0);
	send(instancia_elegida->socket, paquete, tam_paquete, 0);

	log_info(logger, "Espero la respuesta de la Instancia");
	uint32_t respuesta_instancia;
	recv(instancia_elegida->socket, &respuesta_instancia, sizeof(uint32_t), 0);

	if (respuesta_instancia <= 0) {
		log_warning(logger, "La instancia se ha desconectado");
		return -1;
	} else if (respuesta_instancia == PAQUETE_OK) {
		log_info(logger, "La instancia pudo procesar el paquete");
		queue_push(cola_instancias, instancia_elegida); // Lo vuelvo a encolar
	}
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

		// Retardo fictisio
		sleep(retardo * 0.001);

		char* paquete = (char*) malloc(tam_paquete);
		recv(socketESI, paquete, tam_paquete, 0);

		log_info(logger, "Le informo al ESI que el paquete llego correctamente");
		send(socketESI, &PAQUETE_OK, sizeof(uint32_t), 0); // Envio respuesta al ESI

		log_info(logger, "Aguarde mientras se busca una Instancia");
		while (queue_is_empty(cola_instancias)) { // HAY QUE UTILIZAR UN SEMAFORO CONTADOR
			sleep(4); // Lo pongo para que la espera activa no sea tan densa
			log_warning(logger, "No hay instancias disponibles. Reintentando...");
		}

		log_info(logger, "Le envio la instruccion a la Instancia correspondiente");
		int resultadoInstancia = enviarAInstancia(paquete, tam_paquete);
		if (resultadoInstancia < 0) // Instancia desconectada
			// se le avisa al planificador que la instancia no existe mas?
			break;
		/*else
			loguearOperacion(ESI->id, paquete);*/
		destruirPaquete(paquete);
	} while (1);
}

void atenderInstancia(int socketInstancia) {
	t_instancia* unaInstancia = (t_instancia*) malloc(sizeof(t_instancia));
	unaInstancia->id = clave_id;
	clave_id++;
	unaInstancia->socket = socketInstancia;
	unaInstancia->entradas_libres = cant_entradas;
	queue_push(cola_instancias, unaInstancia);
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
			queue_push(cola_instancias, unaInstancia); // Lo vuelvo a encolar
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
	clave_id = 0; // Son unicas

	/*
	 * Se crea en la carpeta Coordinador un archivo "config_coordinador.cfg", la idea es que utilizando la
	 * biblioteca "config.h" se maneje ese CFG con el fin de que el proceso Coordinador obtenga la información
	 * necesaria para establecer su socket. El CFG contiene los campos IP, PUERTO, BACKLOG, PACKAGESIZE.
	 */

	// Importo los datos del archivo de configuracion
	t_config* config = conectarAlArchivo(logger, "/home/utnso/workspace/tp-2018-1c-El-Rejunte/coordinador/config_coordinador.cfg", &error_config);

	ip = obtenerCampoString(logger, config, "IP", &error_config);
	port = obtenerCampoString(logger, config, "PORT", &error_config);
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

	socketDeEscucha = conectarComoServidor(logger, ip, port);

	while (1) { // Infinitamente escucha a la espera de que se conecte alguien
		int socketCliente = escucharCliente(logger, socketDeEscucha);
		pthread_t unHilo; // Cada conexion la delega en un hilo
		pthread_create(&unHilo, NULL, establecerConexion, (void*) &socketCliente);
	}

	finalizarSocket(socketDeEscucha);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

