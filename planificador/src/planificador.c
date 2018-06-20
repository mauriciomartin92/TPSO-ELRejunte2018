#include "planificador.h"

#include "SJF.h"
#include "HRRN.h"

// \n
// todo free de los char*
/* todo ahora hay que adaptar todas las funciones al sistema nuevo de recursos.
 * todo meter los sockets.
 *
 *
 *
 */

int main(void) {

	logPlanificador = log_create("planificador.log", "Planificador" , true, LOG_LEVEL_INFO);
	colaListos = queue_create();
	listaListos = list_create();
	listaFinalizados = list_create();
	deadlockeados = list_create();
	listaRecursos = list_create();


	log_info(logPlanificador,"Arranca el proceso planificador");
	configurar();

	if(string_equals_ignore_case(algoritmoDePlanificacion, SJF) == true)
		{
			log_info(logPlanificador, "la planificacion elegida es SJF");
			planificacionSJF();

		} else {

			log_info(logPlanificador, " la planificacion elegida es HRRN");
			//planficacionHRRN();
		}

	liberarGlobales();

	return EXIT_SUCCESS;

}


void configurar(){

	log_info(logPlanificador, "preparando archivo configuracion");

	archivoConfiguracion = config_create(RUTA_CONFIGURACION);

	log_info (logPlanificador, "preparado");

	algoritmoDePlanificacion = string_new();
	ipCoordinador = string_new();
	ip = string_new();
	puerto = string_new();

	log_info(logPlanificador, "leyendo archivo configuracion ");

	puertoEscucha = config_get_int_value(archivoConfiguracion, KEY_PUERTO_CLIENTE);

	log_info(logPlanificador, "puerto del cliente leido = %d", puertoEscucha);

	string_append( &algoritmoDePlanificacion, config_get_string_value(archivoConfiguracion, KEY_ALGORITMO_PLANIFICACION));

	log_info(logPlanificador, "algoritmo a usar leido = %s", algoritmoDePlanificacion);

	alfa = config_get_int_value(archivoConfiguracion,KEY_CONSTANTE_ESTIMACION);

	log_info(logPlanificador, "constante estimacion leida = %d", alfa);

	estimacionInicial = config_get_int_value(archivoConfiguracion, KEY_ESTIMACION_INICIAL);

	log_info(logPlanificador, "estimacion inicial leida = %d", estimacionInicial);

	string_append(&ipCoordinador,config_get_string_value(archivoConfiguracion, KEY_IP_COORDINADOR));

	log_info(logPlanificador, "ip del coordinador leida = %s", ipCoordinador);

	puertoCoordinador = config_get_int_value(archivoConfiguracion, KEY_PUERTO_COORDINADOR);

	log_info(logPlanificador, "puerto coordinador leido = %d", puertoCoordinador);

	string_append(&ip,config_get_string_value(archivoConfiguracion, KEY_IP));

	log_info(logPlanificador, "mi ip leida = %s", ip);

	string_append(&puerto,config_get_string_value(archivoConfiguracion, KEY_PUERTO));

	log_info(logPlanificador, "mi puerto leido = %s", puerto);

	clavesBloqueadas = config_get_array_value(archivoConfiguracion, KEY_CLAVES_BLOQUEADAS);

	log_info(logPlanificador, "array de claves bloqueadas obtenido");

	log_info(logPlanificador, "llenando lista de bloqueados iniciales");

	int i = 0;

	while (clavesBloqueadas[i] != NULL)
	{

		char ** claves = string_n_split(clavesBloqueadas[i],2,":");
		list_add(listaRecursos,crearRecurso(claves[0]));
		log_info(logPlanificador, " entra a bloqueados el recurso %s", claves[0]);
		crearSubrecurso(claves[0],claves[1]);
		log_info(logPlanificador, "con su subrecurso %s", claves[1]);
		i++;
		free (claves);
	}
	log_info(logPlanificador, "se llenó la cola de bloqueados");

	config_destroy(archivoConfiguracion);
}


ESI * crearESI(char * clave){ // Y EL RECURSO DE DONDE SALE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!?

	ESI * nuevoESI = malloc(sizeof(ESI));
	nuevoESI->id = string_new();
	string_append(&nuevoESI->id, clave);
	nuevoESI->estimacionAnterior= estimacionInicial;
	nuevoESI-> bloqueadoPorUsuario = false;
	nuevoESI-> rafagaAnterior = 0;
	nuevoESI-> estimacionSiguiente = 0;
	nuevoESI->rafagasRealizadas =0;
	nuevoESI-> tiempoEspera = 0;
	nuevoESI->recursoAsignado = NULL;
	nuevoESI->recursoPedido = NULL;

	return nuevoESI;

}


void liberarGlobales (){

	log_info(logPlanificador, "liberando espacio");
	free(algoritmoDePlanificacion);
	free(ipCoordinador);

	int i = 0;
	while(clavesBloqueadas[i]!=NULL)
	{
		free(clavesBloqueadas[i]);
		i++;
	}
	free (clavesBloqueadas);

	log_destroy(logPlanificador);

	list_destroy_and_destroy_elements(listaListos, (void *) ESI_destroy);
	list_destroy_and_destroy_elements(listaFinalizados, (void*) ESI_destroy);
	list_destroy_and_destroy_elements(deadlockeados, (void *) DEADLOCK_destroy);
	queue_destroy_and_destroy_elements(colaListos,(void *)ESI_destroy);
	list_destroy_and_destroy_elements(listaRecursos, (void *) recursoDestroy);
}



