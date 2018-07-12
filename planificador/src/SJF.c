#include "planificador.h"

int socketDeEscucha;
bool planificacionSJFTerminada;

void planificacionSJF(bool desalojo){


	pthread_create(&hiloEscuchaConsola, NULL, (void *) lanzarConsola, NULL);

	pthread_create(&hiloEscuchaESI, NULL, (void *) escucharNuevosESIS, NULL);

	while (queue_size(colaListos) == 0){

		if (queue_size(colaListos) > 0){
			break;
		}

	}

	log_info(logPlanificador, "Comienza planificacion SJF");

	log_info(logPlanificador, "Cola de ESI armada");


	while ( !queue_is_empty(colaListos)){

		bool finalizar = false;

		bool bloquear = false;

		bool permiso = true;

		bool desalojar = false;

		ESI * nuevo = queue_pop(colaListos);

		uint32_t operacion;
		uint32_t tamanioRecurso;
		char * recursoPedido;

		while(!finalizar && !bloquear && permiso && !desalojar){

				while(pausearPlanificacion){} // ciclo hermoso que pausea la planificacion

				send(nuevo->id,&CONTINUAR,sizeof(uint32_t),0);
				int respuesta1 = recv(socketCoordinador, &operacion, sizeof(operacion), 0);
				int respuesta2 = recv(socketCoordinador, &tamanioRecurso, sizeof(uint32_t), 0);
				recursoPedido = malloc(sizeof(tamanioRecurso));
				int respuesta3 = recv(socketCoordinador, recursoPedido, sizeof(char)*tamanioRecurso,0);

				if(respuesta1 < 0 || respuesta2 < 0 || respuesta3 < 0){
					log_info(logPlanificador, "conexion con el coordinador rota");
					exit(-1);
				} else {
					string_append(&nuevo->recursoPedido, recursoPedido);
					nuevo->proximaOperacion = operacion;
					//todo probar que onda con el append si libero el char * que le mete.

				}

				permiso = validarPedido(nuevo->recursoPedido,nuevo);

				if(permiso){

					char * recursoAUsar = nuevo->recursoPedido;

					if(!recursoEnLista(nuevo)){

						list_add(nuevo->recursosAsignado, crearRecurso(nuevo->recursoPedido) );

					}

					bloquearRecurso(recursoAUsar);

					nuevo->recienLlegado = false;
					nuevo->recienDesbloqueadoPorRecurso = false;

					log_info(logPlanificador, " ESI de clave %d entra al planificador", nuevo->id );

					send(socketCoordinador, &CONTINUAR, sizeof(uint32_t),0);

					log_info(logPlanificador, " ejecuta una sentencia ");

					nuevo -> rafagaAnterior = nuevo-> rafagaAnterior +1;
					nuevo -> rafagasRealizadas = nuevo -> rafagasRealizadas +1;

					log_info(logPlanificador, "rafagas realizadas del esi %d son %d", nuevo-> id, nuevo->rafagasRealizadas);

					int respuesta ;
					recv(nuevo->id, &respuesta, sizeof(int),0);

					if (respuesta != CONTINUAR)
					{
						finalizar = true;

					} else if(desalojo) // si hay desalojo activo
					{
						pthread_mutex_lock(&mutexColaListos); // por las dudas que entre otro ESI justo en este momento (no lo tengo en cuenta)

						ESI* auxiliar = queue_peek(colaListos);

						if(auxiliar->recienLlegado) //chequeo si el proximo en cola es un recien llegado
						{
							if(auxiliar->estimacionSiguiente < (nuevo->estimacionSiguiente - nuevo->rafagasRealizadas)) // y si su estimacion siguiente es menor que la del que esta en ejecucion menos lo que ya hizo
							{
								desalojar = true; //se va a desalojar
							} else
							{
								limpiarRecienLlegados(); // Si no es menor la estimacion, los recien llegados ya no tendrian validez, los actualizo
							}
						}
						pthread_mutex_unlock(&mutexColaListos);

					}

					if (nuevo->id == claveParaBloquearESI)
					{
						bloquear = true;
					}



				} else {

					bloquearESI(nuevo->recursoPedido, nuevo);

				}

			}

		log_info(logPlanificador,"fin de ciclo");

		if( finalizar ){ //aca con el mensaje del ESI, determino si se bloquea o se finaliza

			list_add ( listaFinalizados, nuevo);
			log_info(logPlanificador, " ESI de clave %d en finalizados!", nuevo->id);
			liberarRecursos(nuevo);

		} else if( bloquear ){ // este caso sería para bloqueados por usuario. No se libera clave acá

			nuevo->bloqueadoPorUsuario = true;
			bloquearRecurso(claveParaBloquearRecurso);
			bloquearESI(claveParaBloquearRecurso,nuevo);
			log_info(logPlanificador, " ESI de clave %d en bloqueados para recurso %s", nuevo->id, claveParaBloquearESI);

		} else if (desalojar){

			list_add(listaListos,nuevo); //aca no meto mutex porque si llega otro ESI que esté primero que el que genero el desalojo, lo desalojaria igual.
			log_info(logPlanificador," ESI de clave %d desalojado", nuevo->id);
			armarColaListos();

		}

	}

	planificacionSJFTerminada = true;


}

void estimarTiempos(){

	log_info(logPlanificador,"Estimacion de tiempos empezada \n");

	listaListos = list_map(listaListos, (void *)estimarProximaRafaga);

	log_info(logPlanificador,"terminada la estimacion");

}



void armarColaListos(){

	estimarTiempos();

	log_info(logPlanificador, "Tiempos de los ESI listos estimados");

	log_info(logPlanificador,"armando cola de listos");

	ESI * ESIMasRapido;

	while (0 < list_size(listaListos)){

		int i = 1;
		ESIMasRapido = list_get(listaListos, 0);

		while (i < list_size(listaListos) && list_size(listaListos) > 0){

			ESI * ESIAuxiliar = list_get(listaListos, i);
			if (ESIMasRapido->estimacionSiguiente > ESIAuxiliar->estimacionSiguiente){

				ESIMasRapido = ESIAuxiliar;

			} else if (ESIMasRapido->estimacionSiguiente == ESIAuxiliar->estimacionSiguiente) //Ante empate de estimaciones

			{
				if( !ESIAuxiliar->recienLlegado && ESIMasRapido->recienLlegado){ //si no es recien llegado, tiene prioridad porque ya estaba en disco

					ESIMasRapido = ESIAuxiliar;

				} else if( !ESIAuxiliar->recienLlegado && !ESIMasRapido->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIMasRapido->recienDesbloqueadoPorRecurso ){ // si se da que ninguno de los dos recien fue creado, me fijo si alguno se desbloqueo recien de un recurso

					ESIMasRapido = ESIAuxiliar;

				} else if ( ESIAuxiliar->recienLlegado && ESIMasRapido->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIMasRapido->recienDesbloqueadoPorRecurso ){ //si los dos recien llegan, me fijo si el auxiliar recien llego de desbloquearse

					ESIMasRapido = ESIAuxiliar;

				}

			}
			i++;
		}

		log_info(logPlanificador,"ESI numero %d es de estimacion: %d \n", i-1, ESIMasRapido->estimacionSiguiente);

		claveActual = ESIMasRapido->id;
		ESIMasRapido->bloqueadoPorUsuario = false;
		queue_push(colaListos,ESIMasRapido);
		list_remove_by_condition(listaListos, (void *) compararClaves);
	}

	log_info(logPlanificador,"cola armada \n");


}
