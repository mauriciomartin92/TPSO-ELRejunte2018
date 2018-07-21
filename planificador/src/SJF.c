#include "planificador.h"

int socketDeEscucha;
bool planificacionSJFTerminada;

void planificacionSJF(bool desalojo){


	pthread_create(&hiloEscuchaConsola, NULL, (void *) lanzarConsola, NULL);

	pthread_create(&hiloEscuchaESI, NULL, (void *) escucharNuevosESIS, NULL);


	while ( 1 ){

	while (queue_size(colaListos) == 0){

		if (queue_size(colaListos) > 0){
			break;
		}
	}

	while(pausearPlanificacion){} ;

	log_info(logPlanificador, "Comienza planificacion SJF");



	bool finalizar = false;

	bool bloquear = false;

	bool permiso = true;

	bool desalojar = false;


	ESI * nuevo = queue_pop(colaListos);

	claveActual = nuevo->id;

	log_info (logPlanificador, "clave actual ahora es : %d", nuevo->id);

	uint32_t operacion;
	uint32_t tamanioRecurso;
	char * recursoPedido;

	while(!finalizar && !bloquear && permiso && !desalojar && !matarESI){

			while(pausearPlanificacion){} // ciclo hermoso que pausea la planificacion


			pthread_mutex_lock(&mutexComunicacion);
			log_info(logPlanificador, "empieza comunicacion");
			send(nuevo->id,&CONTINUAR,sizeof(uint32_t),0);

			int respuesta1 = recv(socketCoordinador, &operacion, sizeof(operacion), 0);
			int respuesta2 = recv(socketCoordinador, &tamanioRecurso, sizeof(uint32_t), 0);
			recursoPedido = malloc(sizeof(char)*tamanioRecurso);
			int respuesta3 = recv(socketCoordinador, recursoPedido, sizeof(char)*tamanioRecurso,0);

			log_info(logPlanificador, "recibo los datos suficientes para corroborar ejecucion");


			if(respuesta1 <= 0 || respuesta2 <= 0 || respuesta3 <= 0){
				log_info(logPlanificador, "conexion con el coordinador rota");
				liberarGlobales();
				exit(-1);
			} else {
				log_info(logPlanificador, "las respuestas llegaron satisfactoriamente");
				free(nuevo->recursoPedido);
				nuevo->recursoPedido= string_new();
				string_append(&(nuevo->recursoPedido), recursoPedido);
				nuevo->proximaOperacion = operacion;

			}


			permiso = validarPedido(nuevo->recursoPedido,nuevo);

			if(permiso){

				log_info(logPlanificador, "el esi tiene permiso de ejecucion");

				char * valorRecurso;
				uint32_t tamValor;

				if(operacion == 2){

					log_info(logPlanificador, "empieza comunicacion con coordinador para recibir valor (Op: SET)");

					int resp = recv (socketCoordinador, &tamValor,sizeof(uint32_t),0);
					valorRecurso=malloc(sizeof(char)*tamValor);
					int resp2 =recv (socketCoordinador,valorRecurso,sizeof(char)*tamValor,0);

					if(resp <= 0 || resp2 <= 0){

						log_info(logPlanificador, "conexion con el coordinador rota");
						liberarGlobales();
						exit(-1);

					} else {

						log_info(logPlanificador, "llega el valor ");
						cargarValor(recursoPedido,valorRecurso);
						log_info(logPlanificador, "valor cargado");

					}


				}
				pthread_mutex_unlock(&mutexComunicacion);

				if(!recursoEnLista(nuevo)){

					list_add(nuevo->recursosAsignado, recursoPedido );
					log_info(logPlanificador, "El recurso se añadio a los recursos asignados del esi");

				}

				bloquearRecurso(recursoPedido);

				log_info(logPlanificador, "clave bloqueada");


				nuevo->recienLlegado = false;
				nuevo->recienDesbloqueadoPorRecurso = false;

				log_info(logPlanificador, " ESI de clave %d entra al planificador", nuevo->id );

				pthread_mutex_lock(&mutexComunicacion);

				send(socketCoordinador, &CONTINUAR, sizeof(uint32_t),0);

				log_info(logPlanificador, " ejecuta una sentencia ");

				nuevo -> rafagaAnterior = nuevo-> rafagaAnterior +1;
				nuevo -> rafagasRealizadas = nuevo -> rafagasRealizadas +1;

				log_info(logPlanificador, "rafagas realizadas del esi %d son %d", nuevo-> id, nuevo->rafagasRealizadas);

				uint32_t respuesta ;
				int resp =recv(nuevo->id, &respuesta, sizeof(uint32_t),0);
				log_info(logPlanificador, " recibo respuesta del ESI %d, resp: %d",nuevo->id, respuesta);
				pthread_mutex_unlock(&mutexComunicacion);

				if(resp <= 0){

					log_info(logPlanificador, " conexion rota ");
					liberarGlobales();
					exit(-1);
				}
				if (respuesta != CONTINUAR)
				{
					log_info(logPlanificador, " el ESI quiere finalizar ");

					finalizar = true;

				} else if(desalojo && queue_size(colaListos) > 0) // si hay desalojo activo
				{
					log_info(logPlanificador, " hay desalojo ");

					pthread_mutex_lock(&mutexColaListos); // por las dudas que entre otro ESI justo en este momento (no lo tengo en cuenta)

					ESI* auxiliar = queue_peek(colaListos);

					if(auxiliar->recienLlegado) //chequeo si el proximo en cola es un recien llegado
					{
						if(auxiliar->estimacionSiguiente < (nuevo->estimacionSiguiente - nuevo->rafagasRealizadas)) // y si su estimacion siguiente es menor que la del que esta en ejecucion menos lo que ya hizo
						{
							log_info(logPlanificador, "se desaloja el ESI ");

							desalojar = true; //se va a desalojar
						} else
						{
							log_info(logPlanificador, "el esi sigue ejecutando");

							limpiarRecienLlegados(); // Si no es menor la estimacion, los recien llegados ya no tendrian validez, los actualizo
						}
					}
					pthread_mutex_unlock(&mutexColaListos);

				}

				if (nuevo->id == claveParaBloquearESI)
				{
					log_info(logPlanificador, "se pide bloqueo del esi");

					bloquear = true;
				}



			} else {
				log_info(logPlanificador, " El esi no tiene permiso de ejecucion y se bloquea ");

				pthread_mutex_unlock(&mutexComunicacion);
				bloquearESI(nuevo->recursoPedido, nuevo);

			}

		}

		log_info(logPlanificador,"fin de ciclo");

		if(matarESI){

			log_info(logPlanificador, "ESI de clave %d fue mandado a matar por consola", nuevo->id);
			pthread_mutex_lock(&mutexComunicacion);
			log_info(logPlanificador, "le aviso al esi");

			send(nuevo->id,&FINALIZAR,sizeof(uint32_t),0);
			pthread_mutex_unlock(&mutexComunicacion);
			log_info(logPlanificador, "comunicacion terminada");

			log_info(logPlanificador, "se procede a liberar sus recursos y se añade a finalizados");
			liberarRecursos(nuevo);
			list_add(listaFinalizados, nuevo);
			log_info(logPlanificador, "ESI terminado");

	    	pthread_mutex_lock(&mutexAsesino);

	    	matarESI=false;

	    	pthread_mutex_unlock(&mutexAsesino);


		}else if( finalizar ){ //aca con el mensaje del ESI, determino si se bloquea o se finaliza

			list_add ( listaFinalizados, nuevo);
			log_info(logPlanificador, " ESI de clave %d en finalizados!", nuevo->id);
			liberarRecursos(nuevo);
			if(bloquearESIActual) bloquearESIActual = false;


		} else if( bloquear ){ // este caso sería para bloqueados por usuario. No se libera clave acá

			log_info(logPlanificador, "bloqueando esi..");

			nuevo->bloqueadoPorUsuario = true;
			bloquearRecurso(claveParaBloquearRecurso);
			bloquearESI(claveParaBloquearRecurso,nuevo);
			bloquearESIActual = false;
			log_info(logPlanificador, " ESI de clave %d en bloqueados para recurso %s", nuevo->id, claveParaBloquearESI);

		} else if (desalojar){

			armarColaListos(nuevo); //aca no meto mutex porque si llega otro ESI que esté primero que el que genero el desalojo, lo desalojaria igual.
			log_info(logPlanificador," ESI de clave %d desalojado", nuevo->id);


		}

	}

}


void armarColaListos(ESI * esi){

	log_info(logPlanificador, " ESI de ID %d quiere entrar en cola de listos", esi->id);

	estimarProximaRafaga(esi);

	log_info(logPlanificador, "Tiene estimación proxima rafaga: %d", esi->estimacionSiguiente);

	log_info(logPlanificador,"Se procede a meterlo en cola listos ");

	if(queue_size(colaListos) == 0){

		log_info(logPlanificador, "la cola estaba vacía! Entra directo");
		queue_push(colaListos, esi);
		log_info(logPlanificador, "adentro!");

	} else if(queue_size(colaListos) > 0){

		log_info(logPlanificador, " cola llena, procedo a meter el esi de forma ordenada ");
		t_queue * colaAux; // Apunto a cola listos original para ir sacando de a uno de ahi
		t_queue * final = queue_create(); // cola final ordenada
		t_list * llenada = list_create();

		while (0 < queue_size(colaListos)){

			log_info(logPlanificador, " entro a comparar los tiempos");
			ESI * ESIMasRapido = queue_peek(colaListos); // Chusmeo de la cola listos

			colaAux = colaListos;
			int i = 0;
			bool enLista = idEnLista(llenada,ESIMasRapido);

			while (i < queue_size(colaAux) && !enLista){ //voy sacando de a uno

				ESI * ESIAuxiliar = queue_pop(colaAux);

				if(!idEnLista(llenada,ESIAuxiliar)){

					log_info (logPlanificador, "ESI original : %d contra ESI a comparar : %d", ESIMasRapido->estimacionSiguiente, ESIAuxiliar->estimacionSiguiente);

					if (ESIMasRapido->estimacionSiguiente > ESIAuxiliar->estimacionSiguiente){

						log_info(logPlanificador, " el esi a comparar fue se especula que será mas rapido");

						ESIMasRapido = ESIAuxiliar;

					} else if (ESIMasRapido->estimacionSiguiente == ESIAuxiliar->estimacionSiguiente) //Ante empate de estimaciones

					{
						log_info(logPlanificador, "hay empate de estimaciones");

						if( !ESIAuxiliar->recienLlegado && ESIMasRapido->recienLlegado){ //si no es recien llegado, tiene prioridad porque ya estaba en disco

							log_info(logPlanificador, "gana esi nuevo por ser recien llegado");

							ESIMasRapido = ESIAuxiliar;

						} else if( !ESIAuxiliar->recienLlegado && !ESIMasRapido->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIMasRapido->recienDesbloqueadoPorRecurso ){ // si se da que ninguno de los dos recien fue creado, me fijo si alguno se desbloqueo recien de un recurso

							log_info(logPlanificador, "gana esi nuevo por ser recien desbloqueado por recurso");

							ESIMasRapido = ESIAuxiliar;

						} else if ( ESIAuxiliar->recienLlegado && ESIMasRapido->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIMasRapido->recienDesbloqueadoPorRecurso ){ //si los dos recien llegan, me fijo si el auxiliar recien llego de desbloquearse

							log_info(logPlanificador, "gana esi nuevo por ser recien desbloqueado por recurso");

							ESIMasRapido = ESIAuxiliar;

						}

					}
				} log_info(logPlanificador, " el esi para comparar esta en lista, lo salteo ");

				i++;
			}

			if(enLista){

				log_info(logPlanificador, " el ESI que chusmeamos de la cola original estaba en la lista, lo sacamos");
				ESIMasRapido = queue_pop(colaListos);

			}else {

				ESIMasRapido->bloqueadoPorUsuario = false;
				log_info(logPlanificador,"ESI a entrar en la cola es de estimacion: %d",  ESIMasRapido->estimacionSiguiente);
				list_add(llenada, ESIMasRapido);
				queue_push(final, ESIMasRapido); // Meto el ESI mas rapido de los comparados a la cola final

			}

		}


		log_info(logPlanificador,"cola armada ");
		queue_destroy(colaListos);
		list_destroy(llenada);
		colaListos = final;

	}



}


