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
#include "../../biblioteca-El-Rejunte/src/misSockets.h"


typedef enum {
	CIRC,
	LRU,
	BSU
} t_reemplazo;

t_log* logger;
bool error_config;
char* ip_coordinador;
char* port_coordinador;
char* algoritmo_reemplazo;
t_reemplazo protocolo_reemplazo;
uint32_t id_instancia;
int socketCoordinador, intervalo_dump, fd;
uint32_t cant_entradas, tam_entrada, entradas_libres;
t_list* tabla_entradas;
t_list* lista_claves;
//char* mapa_archivo;
char* bloque_instancia;
int puntero_circular;
int pos_a_pisar;
t_instruccion* instruccion; // es la instruccion actual
int referencia_actual = 0;
pthread_mutex_t mutexDumpeo = PTHREAD_MUTEX_INITIALIZER;

const char* ruta_directorio = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/instancia/dump/";
const uint32_t PAQUETE_OK = 1;
const int32_t PAQUETE_ERROR = -1;

void imprimirTablaDeEntradas() {
	printf("\n_______TABLA DE ENTRADAS_______\n");
	for(int i = 0; i < list_size(tabla_entradas); i++) {
		t_entrada* entrada = list_get(tabla_entradas, i);
		printf("%s - %d - %d\n", entrada->clave, entrada->entrada_asociada, entrada->size_valor_almacenado);
	}
	printf("\n");
}

int obtenerEntradasAOcupar(char* valor) {
	div_t division = div(strlen(valor), tam_entrada);
	int entradas_a_ocupar = division.quot;
	if (division.rem > 0) entradas_a_ocupar++;
	return entradas_a_ocupar;
}

int operacion_STORE(char* clave) {
	// Funcion magica para comparar si esta la clave que quiero en la tabla de entradas
	bool comparadorDeClaves(void* estructura) {
		t_entrada* entrada = (t_entrada*) estructura;
		return (strcmp(clave, entrada->clave) == 0);
	}

	// Busco la clave en la tabla usando la funcion magica
	t_entrada* entrada = (t_entrada*) list_find(tabla_entradas, comparadorDeClaves);

	if (dumpearClave(entrada) > 0) {
		log_info(logger, "Se persistió la clave-valor");
		return 1;
	} else {
		log_error(logger, "No se persistió la clave-valor");
		return -1;
	}

	/*char* _nombreArchivo;
	char* _valor;
	int _archivoClave;

	_valor = malloc(sizeof(char) * tam_entrada);
	strncpy(_valor, bloque_instancia+entrada->entrada_asociada, entrada->size_valor_almacenado);
	//_valor[entrada->size_valor_almacenado] = '\0';
	_nombreArchivo = string_new();
	string_append(&_nombreArchivo, ruta_directorio);
	string_append(&_nombreArchivo, entrada->clave);
	string_append(&_nombreArchivo, ".txt");

	//MANEJO DE ERRORES!
	if ((_archivoClave = open(_nombreArchivo, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) > 0) {
		if ((int)write(_archivoClave, _valor, string_length(_valor)) > 0) {
			//SE ESCRIBIÓ PERSISTIÓ LA CLAVE

			log_info(logger, "Se persistió la clave-valor");
			close(_archivoClave);
			return 1;
		} else{
			//MANEJO DE ERRORES!
			log_error(logger, "No se persistió la clave-valor");
			perror("Error:");
			close(_archivoClave);
			return -1;
		}
	} else {
		log_error(logger, "No se pudo abrir el archivo");
		perror("Error:");
		return -1;
	}*/
}

int operacion_SET_reemplazo(t_entrada* entrada, char* valor) {
	int entradas_a_ocupar = obtenerEntradasAOcupar(valor);

	log_debug(logger, "strlen valor: %d - tam entrada: %d", strlen(valor), tam_entrada);
	log_debug(logger, "entradas a ocupar: %d - entradas ocupadas: %d", entradas_a_ocupar, entrada->entradas_ocupadas);

	//Verificamos el tamaño del nuevo valor.
	if (entradas_a_ocupar <= entrada->entradas_ocupadas) {
		//Ocupa lo mismo que el valor anterior.
		liberarEntrada(entrada);
		escribirEntrada(entrada, valor);
		entrada->entradas_ocupadas = entradas_a_ocupar;
		entrada->size_valor_almacenado = strlen(valor);
		entrada->ultima_referencia = referencia_actual;
		actualizarCantidadEntradasLibres();
		log_info(logger, "Se reemplazo el valor de la clave %s", entrada->clave);
		return 1;
	} else {
		log_error(logger, "El valor a registrar supera la cantidad actual de entradas ocupadas por la clave");
		return -1;
	}
		 /*}
		//Ocupa más que el valor anterior.
		int entradas_extra = entradas_a_ocupar - entradas_ocupadas;
		//Libero el valor a reemplazar
		if(entradas_extra <= entradas_libres){
			//Entonces, pregunto si hay entradas libres y contigüas.
			if(1){
				//Escribo
			} else {
				//Compacto el almacenamiento
				//Escribo nuevo valor
			}
		} else {
			//Devuelve la entrada a reemplazar
			t_entrada* entrada = algoritmoDeReemplazo(entradas_a_ocupar);
			//Libero la entrada a reemplazar
			//Entonces, pregunto si hay entradas libres y contigüas.
			if(1){
				//Escribo
			} else {
				//Compacto el almacenamiento
				//Escribo nuevo valor
			}
		}*/

	//Actualizamos el diccionario con el nuevo valor para la clave.
}

t_entrada* algoritmoBSU() {
	bool mayorValorAlmacenado(void* nodo1, void* nodo2) {
		t_entrada* entrada1 = (t_entrada*) nodo1;
		t_entrada* entrada2 = (t_entrada*) nodo2;
		return entrada1->size_valor_almacenado > entrada2->size_valor_almacenado;
	}

	list_sort(tabla_entradas, mayorValorAlmacenado);
	return list_get(tabla_entradas, 0);
}

t_entrada* algoritmoLRU() {
	bool masTiempoReferenciada(void* nodo1, void* nodo2) {
		t_entrada* entrada1 = (t_entrada*) nodo1;
		t_entrada* entrada2 = (t_entrada*) nodo2;
		return entrada1->ultima_referencia > entrada2->ultima_referencia;
	}

	list_sort(tabla_entradas, masTiempoReferenciada);
	return list_get(tabla_entradas, 0);
}

bool buscadorEntradaConPuntero(void* nodo) {
	t_entrada* entrada = (t_entrada*) nodo;
	return (puntero_circular == entrada->entrada_asociada);
}

bool valorEsAtomico(void* nodo) {
	t_entrada* entrada = (t_entrada*) nodo;
	return entrada->entradas_ocupadas == 1;
}

t_entrada* algoritmoCircular() {
	t_entrada* entrada_apuntada = NULL;
	t_list* tabla_entradas_atomicas = list_filter(tabla_entradas, valorEsAtomico);
	while (!entrada_apuntada) {
		entrada_apuntada = list_find(tabla_entradas_atomicas, buscadorEntradaConPuntero);
		puntero_circular++;
	}
	return entrada_apuntada;
}

void algoritmoDeReemplazo() {
	t_entrada* entrada_a_reemplazar;

	switch (protocolo_reemplazo) {
	case LRU:
		entrada_a_reemplazar = algoritmoLRU();
		break;

	case BSU:
		entrada_a_reemplazar = algoritmoBSU();
		break;

	default: // Circular
		entrada_a_reemplazar = algoritmoCircular();
	}

	liberarEntrada(entrada_a_reemplazar);
	actualizarCantidadEntradasLibres();
	log_info(logger, "Se ha liberado la entrada %d de clave %s", entrada_a_reemplazar->entrada_asociada, entrada_a_reemplazar->clave);
}

int operacion_SET(t_instruccion* instruccion) {
	int entradas_a_ocupar = obtenerEntradasAOcupar(instruccion->valor);

	log_info(logger, "El valor a almacenar requiere %i entradas", entradas_a_ocupar);

	if (entradas_a_ocupar <= entradas_libres) {
		// Entonces, pregunto si hay entradas libres y contiguas.
		log_info(logger, "Hay %d entradas libres para almacenar el valor", entradas_a_ocupar);
		if (hayEntradasContiguas(entradas_a_ocupar) >= 0) {
			log_info(logger, "Hay %d entradas contiguas", entradas_a_ocupar);
		} else {
			log_info(logger, "No hay %d entradas contiguas para almacenar el valor", entradas_a_ocupar);
			compactarAlmacenamiento();
		}
	} else {
		// Devuelve la entrada a reemplazar
		log_info(logger, "No hay %d entradas para almacenar el valor, aplico algoritmo de reemplazo %s", entradas_a_ocupar, algoritmo_reemplazo);

		while (entradas_a_ocupar > entradas_libres) algoritmoDeReemplazo();
		log_info(logger, "Hay %d entradas libres para almacenar el valor", entradas_a_ocupar);
		// Entonces, pregunto si hay contiguas.
		if (hayEntradasContiguas(entradas_a_ocupar) >= 0) {
			log_info(logger, "Hay %d entradas contiguas", entradas_a_ocupar);
		} else {
			log_info(logger, "No hay %d entradas contiguas para almacenar el valor", entradas_a_ocupar);
			compactarAlmacenamiento();
		}
	}

	t_entrada* entrada = (t_entrada*) malloc(sizeof(t_entrada));
	entrada->clave = string_new();
	string_append(&(entrada->clave), instruccion->clave);
	entrada->size_valor_almacenado = strlen(instruccion->valor);
	entrada->entradas_ocupadas = entradas_a_ocupar;
	entrada->ultima_referencia = referencia_actual;
	entrada->entrada_asociada = hayEntradasContiguas(entradas_a_ocupar);

	escribirEntrada(entrada, instruccion->valor);
	actualizarCantidadEntradasLibres();

	list_add(tabla_entradas, entrada);
	log_info(logger, "Se ha escrito la entrada");

	return 1;
}

int validarArgumentosInstruccion(t_instruccion* instruccion) {
	log_info(logger, "Validando que la instruccion sea ejecutable...");

	printf("La instruccion recibida es: ");
	switch (instruccion->operacion) {
	case opGET:
		printf("GET %s\n", instruccion->clave);
		log_error(logger, "Una Instancia no puede ejecutar GET");
		send(socketCoordinador, &PAQUETE_ERROR, sizeof(uint32_t), 0);
		return -1;
		break;

	case opSET:
		printf("SET %s %s\n", instruccion->clave, instruccion->valor);
		break;

	case opSTORE:
		printf("STORE %s\n", instruccion->clave);
		break;

	default:
		log_error(logger, "No comprendo la instruccion, le informo al Coordinador que no se puede ejecutar");
		send(socketCoordinador, &PAQUETE_ERROR, sizeof(uint32_t), 0);
		return -1;
	}

	log_info(logger, "Le informo al Coordinador que el paquete llego correctamente");
	send(socketCoordinador, &PAQUETE_OK, sizeof(uint32_t), 0);
	return 1;
}

void actualizarEntradaAsociada(void* nodo) {
	t_entrada* entrada = (t_entrada*) nodo;
	if ((entrada->entrada_asociada * tam_entrada) > pos_a_pisar) {
		entrada->entrada_asociada--;
	}
}

void compactarAlmacenamiento() {
	log_info(logger, "Se inicia la compactacion del almacenamiento");

	for (int x = 0; x < cant_entradas * tam_entrada; x += tam_entrada) {
		if (bloque_instancia[x] == '0') {
			for (int y = x + tam_entrada; x < cant_entradas * tam_entrada; y += tam_entrada) {
				if (bloque_instancia[y] != '0') {
					char* porcion = string_new();
					for (int z = y; z < cant_entradas * tam_entrada; z++) {
						string_append_with_format(&porcion, "%c", bloque_instancia[z]);
					}
					while ((x + strlen(porcion)) < strlen(bloque_instancia)) {
						string_append_with_format(&porcion, "%c", '0'); // Se agregan los '0' al final ya que strncpy no lo hace
					}
					strncpy(bloque_instancia + x, porcion, strlen(porcion)); // Pisa la entrada vacia con el bloque posterior
					pos_a_pisar = x; // Posicion respecto a la que se deben actualizar las entradas asociadas

					list_map(tabla_entradas, actualizarEntradaAsociada); // Se actualizan las entradas asociadas si corresponde

					break;
				}
			}
		}
	}
	log_info(logger, "Se compactaron todas las entradas");
}

int dumpearClave(void* nodo) {
	t_entrada* entrada = (t_entrada*) nodo;
	char* _nombreArchivo = string_new();
	string_append(&_nombreArchivo, ruta_directorio);
	string_append(&_nombreArchivo, entrada->clave);
	string_append(&_nombreArchivo, ".txt");

	int _fd = open(_nombreArchivo, O_RDWR, S_IRUSR | S_IWUSR);
	if (_fd < 0) { // Si el archivo no existia => lo crea, y crea el mapa
		_fd = open(_nombreArchivo, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if (_fd < 0) {
			log_error(logger, "El archivo no se puede crear");
			return -1;
		}
		ftruncate(_fd, entrada->size_valor_almacenado);
		entrada->fd = _fd;
		entrada->mapa_archivo = mmap(NULL, entrada->size_valor_almacenado, PROT_READ | PROT_WRITE, MAP_SHARED, entrada->fd, 0);

		int pos_inicial = (entrada->entrada_asociada - 1) * tam_entrada;
		for (int i = pos_inicial; i < pos_inicial + entrada->size_valor_almacenado; i++) {
			entrada->mapa_archivo[i - pos_inicial] = bloque_instancia[i];
		}
		//msync(NULL, entrada->size_valor_almacenado, MS_SYNC);
	} else {
		if (entrada->size_valor_almacenado <= strlen(entrada->mapa_archivo)) {
			memset(entrada->mapa_archivo, '0', strlen(entrada->mapa_archivo));
			strncpy(entrada->mapa_archivo, bloque_instancia + ((entrada->entrada_asociada - 1) * tam_entrada), strlen(entrada->mapa_archivo));
		} else {
			ftruncate(entrada->fd, entrada->size_valor_almacenado);
			munmap(entrada->mapa_archivo, strlen(entrada->mapa_archivo));
			entrada->mapa_archivo = string_new();
			entrada->mapa_archivo = mmap(NULL, entrada->size_valor_almacenado, PROT_READ | PROT_WRITE, MAP_SHARED, entrada->fd, 0);
			//entrada->mapa_archivo = mremap(NULL, strlen(entrada->mapa_archivo), entrada->size_valor_almacenado, 0);
		}
	}

	/*if (_fd < 0) {
		log_error(logger, "Error al abrir archivo para DUMP");
		return -1;
	} else {
		if(sb.st_size > 0) {
			printf("hay algo en el archivo...\n");
			if (entrada->size_valor_almacenado <= strlen(entrada->mapa_archivo)) {
				printf("tamaño nuevo valor es menor o igual que el almacenado\n");
				memset(entrada->mapa_archivo, '0', strlen(entrada->mapa_archivo));
				strncpy(entrada->mapa_archivo, bloque_instancia + ((entrada->entrada_asociada - 1) * tam_entrada), strlen(entrada->mapa_archivo));
			} else {
				printf("tamaño nuevo valor es mayor que el almacenado\n");
				ftruncate(entrada->fd, entrada->size_valor_almacenado);
				munmap(entrada->mapa_archivo, strlen(entrada->mapa_archivo));
				entrada->mapa_archivo = string_new();
				entrada->mapa_archivo = mmap(NULL, entrada->size_valor_almacenado, PROT_READ | PROT_WRITE, MAP_SHARED, entrada->fd, 0);
				//entrada->mapa_archivo = mremap(NULL, strlen(entrada->mapa_archivo), entrada->size_valor_almacenado, 0);
			}
		} else {
			puts("el archivo no existe, hay que crearlo");
			entrada->fd = _fd;
			entrada->mapa_archivo = mmap(NULL, entrada->size_valor_almacenado, PROT_READ | PROT_WRITE, MAP_SHARED, entrada->fd, 0);
			entrada->mapa_archivo = string_substring(bloque_instancia, (entrada->entrada_asociada - 1) * tam_entrada, entrada->size_valor_almacenado);
		}
		close(_fd);
	}*/
	return 1;
}

void* dumpAutomatico() {
	while(1){
		sleep(intervalo_dump);
		pthread_mutex_lock(&mutexDumpeo);
		printf("\n...EJECUTANDO DUMP AUTOMATICO...\n");
		list_iterate(tabla_entradas, dumpearClave);
		printf("...FIN DUMP AUTOMATICO...\n\n");
		pthread_mutex_unlock(&mutexDumpeo);
	}
	return NULL;
}

void escribirEntrada(t_entrada* entrada, char* valor) {
	//strncpy(bloque_instancia + entrada->entrada_asociada, valor, entrada->entradas_ocupadas * tam_entrada);
	int pos_inicial = (entrada->entrada_asociada - 1) * tam_entrada;
	for (int i = pos_inicial; i < pos_inicial + strlen(valor); i++) {
		bloque_instancia[i] = valor[i - pos_inicial];
	}
}

t_entrada* crearEntradaDesdeArchivo(char* archivo) {
	log_debug(logger, "%s", archivo);
	struct stat sb;
	t_entrada* entrada = (t_entrada*) malloc(sizeof(t_entrada));

	entrada->clave = string_new();
	char** vector_clave = string_split(archivo, ".");
	string_append(&(entrada->clave), vector_clave[0]);

	entrada->path = string_new();
	string_append(&entrada->path, ruta_directorio);
	string_append(&entrada->path, archivo);
	log_debug(logger, "%s", entrada->path);

	entrada->fd = open(entrada->path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	fstat(entrada->fd, &sb);

	entrada->mapa_archivo = string_new();
	entrada->mapa_archivo = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, entrada->fd, 0);

	entrada->size_valor_almacenado = sb.st_size;

	llenarAlmacenamiento(entrada);

	entrada->ultima_referencia = referencia_actual;

	return entrada;
}

int iniciarDirectorio(){
	DIR* dirp;
	struct dirent *dp;
	char* archivos;
	char** vector_archivos;

	tabla_entradas = list_create();

	dirp = opendir(ruta_directorio);
	if (!dirp) {
		log_error(logger, "No se encontro el directorio de valores persistidos, se aborta la Instancia");
		return -1;
	}

	archivos = string_new();

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_type == 8) { // Si es .txt
			string_append(&archivos, "-");
			string_append(&archivos, dp->d_name);
		}
	}

	if (string_length(archivos) == 0) {
		closedir(dirp);
		return 1;
	}

	archivos = string_substring_from(archivos, 1);
	vector_archivos = string_split(archivos, "-");

	int i = 0;
	while(vector_archivos[i] != NULL){
		log_debug(logger, "%s", vector_archivos[i]);
		list_add(tabla_entradas, crearEntradaDesdeArchivo(vector_archivos[i]));
		i++;
	}
	closedir(dirp);
	return 1;
}

void llenarAlmacenamiento(t_entrada* entrada) {
	int entradas_a_ocupar = obtenerEntradasAOcupar(entrada->mapa_archivo);

	for (int i = 0; i < cant_entradas * tam_entrada; i += tam_entrada) {
		if (bloque_instancia[i] == '0') {
			log_debug(logger, "mapa: %s", entrada->mapa_archivo);
			strncpy(bloque_instancia + i, entrada->mapa_archivo, strlen(entrada->mapa_archivo));
			entrada->entrada_asociada = (i / tam_entrada) + 1;
			entrada->entradas_ocupadas = entradas_a_ocupar;
			log_debug(logger, "%s", bloque_instancia);
			break;
		}
	}
	bloque_instancia[cant_entradas * tam_entrada] = '\0';
}

int hayEntradasContiguas(int entradas_necesarias){
	int contador = 0;
	int primer_entrada;

	for (int i = 0; i < tam_entrada * cant_entradas; i = i + tam_entrada){
		if(bloque_instancia[i] == '0'){
			contador++;
			if(contador == 1){
				primer_entrada = i;
			}
			if(contador == entradas_necesarias){
				return (primer_entrada / tam_entrada) + 1;
			}
		} else {
			contador = 0;
			primer_entrada = 0;
		}
	}
	return -1;
}

void liberarEntrada(t_entrada* entrada) {
	//strncpy(bloque_instancia+entrada->entrada_asociada, '0', entrada->entradas_ocupadas*tam_entrada);
	memset(bloque_instancia + (entrada->entrada_asociada - 1) * tam_entrada, '0', entrada->entradas_ocupadas * tam_entrada);
}

void actualizarCantidadEntradasLibres() {
	int cont = 0;

	// Recorre el almacenamiento por cada entrada, preguntando si el primer valor en c/u es nulo.
	for (int i = 0; i < tam_entrada * cant_entradas; i = i + tam_entrada) {
		if (bloque_instancia[i] == '0') {
			cont++;
		}
	}

	entradas_libres = cont;
}

void inicializarBloqueInstancia() {
	bloque_instancia = (char*) malloc(sizeof(char) * ((cant_entradas * tam_entrada) + 1));
	memset(bloque_instancia, '0', cant_entradas * tam_entrada);
	bloque_instancia[cant_entradas * tam_entrada] = '\0';
}

int procesar(t_instruccion* instruccion) {
	log_info(logger, "Procesando instruccion...");

	// Funcion magica para comparar si esta la clave que quiero en la tabla de entradas
	bool comparadorDeClaves(void* estructura) {
		t_entrada* entrada = (t_entrada*) estructura;
		return (strcmp(instruccion->clave, entrada->clave) == 0);
	}

	// Busco la clave en la tabla usando la funcion magica
	t_entrada* entrada = (t_entrada*) list_find(tabla_entradas, comparadorDeClaves);

	// Evaluo como procesar segun las condiciones
	if (!entrada) { // la entrada no estaba
		log_warning(logger, "La clave no esta registrada en la Tabla de Entradas");

		switch (instruccion->operacion) {
		case opSET: // SET de clave ausente
			log_debug(logger, "Operacion SET de clave nueva");
			return operacion_SET(instruccion);
		case opSTORE: // STORE de clave ausente
			log_error(logger, "Intento de STORE para una clave inexistente");
			return 1;
		}
	} else { // la entrada si estaba
		log_warning(logger, "La clave esta registrada en la Tabla de Entradas");

		switch (instruccion->operacion){
		case opSET: // SET de clave presente.
			log_debug(logger, "Operacion SET con reemplazo");
			return operacion_SET_reemplazo(entrada, instruccion->valor);
		case opSTORE: // STORE de clave presente.
			log_debug(logger, "Operacion STORE para clave existente");
			return operacion_STORE(instruccion->clave);
		}
	}
	return -1;
}

t_instruccion* recibirInstruccion(int socketCoordinador) {
	// Recibo linea de script parseada
	uint32_t tam_paquete;
	recv(socketCoordinador, &tam_paquete, sizeof(uint32_t), 0); // Recibo el header

	char* paquete = (char*) malloc(sizeof(char) * tam_paquete);
	recv(socketCoordinador, paquete, tam_paquete, 0);
	log_info(logger, "Recibi un paquete que me envia el Coordinador");
	log_debug(logger, "%s", paquete);

	t_instruccion* instruccion = desempaquetarInstruccion(paquete, logger);
	//destruirPaquete(paquete);

	return instruccion;
}

// Protocolo numerico de ALGORITMO_DISTRIBUCION
void establecerProtocoloReemplazo() {
	if (strcmp(algoritmo_reemplazo, "LRU") == 0) {
		protocolo_reemplazo = LRU;
	} else if (strcmp(algoritmo_reemplazo, "BSU") == 0) {
		protocolo_reemplazo = BSU;
	} else {
		protocolo_reemplazo = CIRC;
		puntero_circular = 1;
	}
}

t_control_configuracion cargarConfiguracion() {
	// Importo los datos del archivo de configuracion
	t_config* config = conectarAlArchivo(logger, "/home/utnso/workspace/tp-2018-1c-El-Rejunte/instancia/config_instancia.cfg", &error_config);

	ip_coordinador = obtenerCampoString(logger, config, "IP_COORDINADOR", &error_config);
	port_coordinador = obtenerCampoString(logger, config, "PORT_COORDINADOR", &error_config);
	algoritmo_reemplazo = obtenerCampoString(logger, config, "ALGORITMO_REEMPLAZO", &error_config);
	id_instancia = obtenerCampoInt(logger, config, "ID_INSTANCIA", &error_config);
	intervalo_dump = obtenerCampoInt(logger, config, "INTERVALO_DUMP", &error_config);

	establecerProtocoloReemplazo();

	// Valido si hubo errores
	if (error_config) {
		log_error(logger, "No se pudieron obtener todos los datos correspondientes");
		return CONFIGURACION_ERROR;
	}
	return CONFIGURACION_OK;
}

void finalizar() {
	if (socketCoordinador > 0) finalizarSocket(socketCoordinador);
	list_destroy(tabla_entradas);
	log_destroy(logger);
	//munmap(mapa_archivo, sizeof(mapa_archivo));
	close(fd);
	free(bloque_instancia);
	free(tabla_entradas);
}

int main() {
	error_config = false;

	// Creo el logger
	logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL_DEBUG);

	if (cargarConfiguracion() == CONFIGURACION_ERROR) {
		log_error(logger, "No se pudo cargar la configuracion");
		finalizar(); // Si hubo error, se corta la ejecucion.
		return EXIT_FAILURE;
	}

	// Me conecto con el Servidor y le mando mensajes
	socketCoordinador = conectarComoCliente(logger, ip_coordinador, port_coordinador);
	uint32_t handshake = INSTANCIA;
	send(socketCoordinador, &handshake, sizeof(uint32_t), 0);

	// Le aviso cual es mi ID
	send(socketCoordinador, &id_instancia, sizeof(uint32_t), 0);

	recv(socketCoordinador, &cant_entradas, sizeof(uint32_t), 0);
	recv(socketCoordinador, &tam_entrada, sizeof(uint32_t), 0);

	log_debug(logger, "cant entradas: %d\n", cant_entradas);
	log_debug(logger, "tam entrada: %d\n", tam_entrada);

	log_info(logger, "Se recibio la cantidad y tamaño de las entradas correctamente");

	inicializarBloqueInstancia();
	if (iniciarDirectorio() < 0) {
		finalizar();
		return EXIT_FAILURE;
	}
	if (list_size(tabla_entradas) > 0) imprimirTablaDeEntradas();

	//Generamos temporizador
	pthread_t hiloTemporizador;
	//pthread_create(&hiloTemporizador, NULL, dumpAutomatico, NULL);

	actualizarCantidadEntradasLibres();

	while (1) {
		log_debug(logger, "Cantidad de entradas libres: %d", entradas_libres);
		log_debug(logger, "%s", bloque_instancia);
		log_debug(logger, "TAMAÑO BLOQUE: %d", strlen(bloque_instancia));

		t_instruccion* instruccion = recibirInstruccion(socketCoordinador);
		if (validarArgumentosInstruccion(instruccion) > 0) {

			referencia_actual++;

			pthread_mutex_lock(&mutexDumpeo);
			int resultado = procesar(instruccion);
			pthread_mutex_unlock(&mutexDumpeo);

			if (resultado > 0) {
				log_info(logger, "Le aviso al Coordinador que se proceso la instruccion");
				/*/char** para_imprimir = string_split(mapa_archivo, ";");
				int i = 0;
				while (para_imprimir[i] != NULL) {
					printf("%s\n", para_imprimir[i]);
					i++;
				}*/
				send(socketCoordinador, &entradas_libres, sizeof(uint32_t), 0);
			} else {
				log_error(logger, "Le aviso al Coordinador que no se pudo procesar la instrucción");
				send(socketCoordinador, &PAQUETE_ERROR, sizeof(uint32_t), 0);
			}

			if (instruccion->operacion == 2) printf("\x1b[34m	INSTRUCCION:%d %s %s\x1b[0m\n", instruccion->operacion, instruccion->clave, instruccion->valor);
			if (instruccion->operacion == 3) printf("\x1b[34m	INSTRUCCION:%d %s\x1b[0m\n", instruccion->operacion, instruccion->clave);
		}
	}
	finalizar();
	return EXIT_SUCCESS;
}
