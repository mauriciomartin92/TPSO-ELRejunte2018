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

t_esi_operacion parsearLineaScript(FILE* fp) {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	read = getline(&line, &len, fp);
	printf("%s", line);
	t_esi_operacion parsed = parse(line);

	/*while ((read = getline(&line, &len, fp)) != -1) {
	 t_esi_operacion parsed = parse(line);

	 if (parsed.valido) {
	 switch (parsed.keyword) {
	 case GET:
	 printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
	 break;
	 case SET:
	 printf("SET\tclave: <%s>\tvalor: <%s>\n",
	 parsed.argumentos.SET.clave,
	 parsed.argumentos.SET.valor);
	 break;
	 case STORE:
	 printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
	 break;
	 default:
	 fprintf(stderr, "No pude interpretar <%s>\n", line);
	 exit(EXIT_FAILURE);
	 }

	 destruir_operacion(parsed);
	 } else {
	 fprintf(stderr, "La linea <%s> no es valida\n", line);
	 exit(EXIT_FAILURE);
	 }
	 }*/

	if (line)
		free(line);

	return parsed;
}

int main() {
	char* ip;
	char* port;
	int packagesize;
	bool error_config = false;

	logger = log_create("esi.log", "ESI", true, LOG_LEVEL_INFO);

	// Se crea una estructura de datos que contendra todos lo datos de mi CFG que lea la funcion config_create
	t_config* config = conectarAlArchivo(logger, "../config_esi.cfg",
			&error_config);

	ip = obtenerCampoString(logger, config, "IP_PLANIFICADOR", &error_config);
	port = obtenerCampoString(logger, config, "PORT_PLANIFICADOR",
			&error_config);
	packagesize = obtenerCampoInt(logger, config, "PACKAGESIZE", &error_config);

	if (!error_config) {
		log_info(logger, "ENCONTRO LOS DATOS DE CONFIG !!!");
	} else {
		log_error(logger, "NO SE PUDO CONECTAR CORRECTAMENTE.");
		//return EXIT_FAILURE; // Si hubo error, se corta la ejecucion.
	}

	// Abro el fichero del script
	FILE *fp;
	fp = fopen("../script.esi", "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	int socketPlanificador = conectarComoCliente(logger, ip, port);
	recibirMensaje(logger, socketPlanificador, packagesize);

	if (!feof(fp))
		t_esi_operacion lineaParseada = parsearLineaScript(fp);

	fclose(fp);
	return EXIT_SUCCESS;
}
