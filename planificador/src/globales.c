#include "planificador.h"


//char* rutaLog = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/Debug/logs";
char * KEY_PUERTO_CLIENTE = "PUERTO_CLIENTE";
char * KEY_ALGORITMO_PLANIFICACION = "ALGORITMO_PLANIFICACION";
char * KEY_ESTIMACION_INICIAL = "ESTIMACION_INICIAL";
char * KEY_IP_COORDINADOR = "IP_COORDINADOR";
char * KEY_PUERTO_COORDINADOR = "PUERTO_COORDINADOR";
char * KEY_IP = "IP";
char * KEY_PUERTO = "PUERTO";
char * KEY_CLAVES_BLOQUEADAS = "CLAVES_BLOQUEADAS";
char * KEY_CONSTANTE_ESTIMACION = "CONSTANTE_ESTIMACION";
char * RUTA_CONFIGURACION = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/config_planificador.cfg";
char * SJF = "SJF";
char * HRRN = "HRRN";
char * ipPropia = "127.0.0.2";
char * puertoPropio = "8080";
int CONTINUAR = 1;
int FINALIZAR = 2;


// CONSOLA


char * PAUSEAR_PLANIFICACION= "pausear_planificacion";
char* REANUDAR_PLANIFICACION = "reanudar_planificacion";
char* BLOQUEAR_ESI = "bloquear_esi";
char* DESBLOQUEAR_ESI = "desbloquear_esi";
char* LISTAR_POR_RECURSO = "listar_por_recurso";
char* KILL_ESI = "kill_esi";
char* STATUS_ESI = "status_esi";
char* COMPROBAR_DEADLOCK = "comprobar_deadlock";




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


void lanzarConsola(){

	char* linea;

	while(1){

		printf("¿Que operacion desea hacer?\n");
		printf("Pausear_planificacion \n");
		printf("Reanudar_planificacion \n");
		printf("Bloquear_ESI \n");
		printf("Desbloquear_ESI \n");
		printf("Listar_por_recurso \n");
		printf("Kill_ESI \n");
		printf("Status_ESI \n");
		printf("Comprobar_Deadlock \n");
		printf("Salir");
		printf("Ingrese el nombre de la opcion, incluyendo el guion bajo");
		linea = readline(">");

		if (string_equals_ignore_case(linea, PAUSEAR_PLANIFICACION))  //Hermosa cadena de if que se viene
		{
			//todo funcion que pausee
			// Conectar con planificador e indicarle que el ESI se pausea pero NO se bloquea, solo para temporalmente la planificacion
			break;
		}
		else if (string_equals_ignore_case(linea,REANUDAR_PLANIFICACION))
		{
			//todo funcion que reanude
			//reanuda lo pauseado -> podrian ser la misma opcion con pausear porque la comprobacion se hace igual
			break;
		}
		else if (string_equals_ignore_case(linea, BLOQUEAR_ESI))
		{
			//todo funcion que bloquee
			linea = readline("CLAVE:");
			//mandar clave al planificador
			// Manda al ESI a la cola de bloqueados del recurso CLAVE si el mismo esta en ejecucion o listo para ejecurar
			break;
		}
		else if (string_equals_ignore_case(linea,DESBLOQUEAR_ESI))
		{
			//todo funcion que desbloquee al primer bloqueado de la cola del recurso
			linea = readline("CLAVE:");
			break;
		}
		else if (string_equals_ignore_case(linea, LISTAR_POR_RECURSO)){

			//todo funcion que liste
			linea = readline("RECURSO:");
			//Aca agarra la clave del recurso a usar y muestra por pantalla los recursos que estan esperando usarlo
			break;
		}
		else if (string_equals_ignore_case(linea, KILL_ESI))
		{
			//Mata al ESI y libera sus recursos
			//todo funcion asesina
			linea = readline("CLAVE:");
			break;
		}
		else if (string_equals_ignore_case(linea, STATUS_ESI))
		{
			//todo funcion explicacion pendiente
			linea = readline("CLAVE:");
			//Explicacion pendiente
			break;
		}
		else if (string_equals_ignore_case(linea, COMPROBAR_DEADLOCK))
		{
			//devuelve las claves que esten generando deadlock (si es que lo hay)
			// aca va a haber una escucha del socket a la espera de la respuesta del plani
			break;
		}
		else if (string_equals_ignore_case(linea, "salir"))
		{
			break;
		}
		else
		{
			printf("escribi bien, gil");
			break;
		}

	}
}



t_recurso * crearRecurso (char * id){

	t_recurso * nuevo = malloc(sizeof(t_recurso));
	nuevo->clave = id;
	nuevo->subrecursos = list_create();
	return nuevo;

}

void crearSubrecurso (char* claveRecurso, char * claveSubrecurso)
{

	t_subrecurso * nuevoSubrecurso = malloc (sizeof(t_subrecurso));
	nuevoSubrecurso->clave = claveSubrecurso;
	nuevoSubrecurso->recursosFinales = list_create();

	int i = 0;
	bool encontrado = false;
	while(list_size(listaRecursos) >= i)
	{
		if(string_equals_ignore_case(list_get(listaRecursos,i),claveRecurso))
		{
			t_recurso * auxiliar = list_get(listaRecursos,i);
			list_add(auxiliar->subrecursos,nuevoSubrecurso);
			list_replace_and_destroy_element(listaRecursos, i, auxiliar, (void *) recursoDestroy);
			encontrado = true;
		}
	}

	if(encontrado == false){

		t_recurso * nuevoRecurso = crearRecurso(claveRecurso);
		list_add( nuevoRecurso->subrecursos, nuevoSubrecurso);
	}

}


void recursoDestroy(t_recurso * recurso){

	free(recurso->clave);
	list_destroy_and_destroy_elements(recurso->subrecursos, (void *) subrecursoDestroy);

}

void subrecursoDestroy (t_subrecurso * subrecurso){

	free (subrecurso-> clave);
	queue_destroy_and_destroy_elements(subrecurso->ESIEncolados, (void*) ESI_destroy);
	list_destroy_and_destroy_elements(subrecurso->recursosFinales, (void *)recursoFinalDestroy);

}

void recursoFinalDestroy(t_recursoFinal * recuFinal){

	free(recuFinal->clave);
	free(recuFinal->valor);

}

void ESI_destroy(ESI * estructura)
{
	free(estructura->id);
	free(estructura->recursoAsignado);
	free(estructura->recursoPedido);
	free(estructura);

}

void DEADLOCK_destroy(t_deadlockeados * ESI){

	free(ESI->clave);
	list_destroy_and_destroy_elements(ESI->ESIasociados, (void *) free);


}


//todo esperar respuesta de ayudante.
void bloquearSubrecurso (char* claveRecurso, char * claveESI) // agarra la clave entera del recurso (futbol:messi), encuentra el subrecurso y mete el ESI que se bloquea en su cola de bloqueados
{

	//char ** claves = string_n_split(claveRecurso,2,":");
	t_subrecurso * nuevoSubrecurso = malloc (sizeof(t_subrecurso));

	int i = 0;
	bool encontrado = false;
	while(list_size(listaRecursos) >= i)
	{
		if(string_equals_ignore_case(list_get(listaRecursos,i),claveRecurso))
		{
			t_recurso * auxiliar = list_get(listaRecursos,i);
			list_add(auxiliar->subrecursos,nuevoSubrecurso);
			list_replace_and_destroy_element(listaRecursos, i, auxiliar, (void *) recursoDestroy);
			encontrado = true;
		}
	}

	if(encontrado == false){

		t_recurso * nuevoRecurso = crearRecurso(claveRecurso);
		list_add( nuevoRecurso->subrecursos, nuevoSubrecurso);
	}

}


