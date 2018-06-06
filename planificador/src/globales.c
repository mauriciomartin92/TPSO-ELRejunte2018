#include "Planificador.h"


char* rutaLog = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/logs";
char * KEY_PUERTO_CLIENTE = "PUERTO_CLIENTE";
char * KEY_ALGORITMO_PLANIFICACION = "ALGORITMO_PLANIFICACION";
char * KEY_ESTIMACION_INICIAL = "ESTIMACION_INICIAL";
char * KEY_IP_COORDINADOR = "IP_COORDINADOR";
char * KEY_PUERTO_COORDINADOR = "PUERTO_COORDINADOR";
char * KEY_CLAVES_BLOQUEADAS = "CLAVES_BLOQUEADAS";
char * KEY_CONSTANTE_ESTIMACION = "CONSTANTE_ESTIMACION";
char * RUTA_CONFIGURACION = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/src/Planificador.cfg";
char * SJF = "SJF";
char * HRRN = "HRRN";


void estimarProximaRafaga(ESI * proceso ){


	log_info (logPlanificador, "EL ESI DE CLAVE %s TIENE RAFAGA ANTERIOR DE %d", proceso->id, proceso->rafagaAnterior);
	if(proceso -> rafagaAnterior == 0){

		proceso -> estimacionSiguiente = 0;

	} else{

		proceso->estimacionSiguiente = ((alfa*proceso->estimacionAnterior)+(1-alfa)*proceso->rafagaAnterior);

	}
	log_info(logPlanificador,"un tiempo estimado \n");

}


bool compararClaves (ESI * esi){

		log_info(logPlanificador, "entra a la comparacion de claves");

		if(string_equals_ignore_case(esi->id, claveActual) == true){

			return true;

		} else{

			return false;

		}

	}

void comprobarDeadlock(){ //todo preguntar si el deadlock solo es por espera circular o influyen planificaciones y tiempos

	list_clean_and_destroy_elements(deadlockeados, (void *) DEADLOCK_destroy);

	int contador = 0;

	while ( contador <= list_size(listaListos) ){

		ESI * nuevo = list_get(listaListos, contador);
		t_deadlockeados * posibleDeadlock = malloc (sizeof(deadlockeados));

		int contador2;

		while (contador2 <= list_size(listaListos)){

			ESI * Aux = list_get(listaListos, contador2);
			if ( (Aux -> recursoPedido == nuevo -> recursoAsignado) && (Aux -> recursoAsignado == nuevo -> recursoPedido)){

				posibleDeadlock->clave = nuevo->id;
				list_add(posibleDeadlock->ESIasociados, Aux->id);
			}
			ESI_destroy(Aux);
			contador2++;

		}
		ESI_destroy(nuevo);
		DEADLOCK_destroy(posibleDeadlock);
		contador++;


	} //todo estoy metiendo repetidos los deadlock porque no compruebo si la clave ya está dentro de otra estructura deadlock. ARREGLAR




}

