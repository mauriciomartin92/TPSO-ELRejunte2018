#include "SJF.h"

int socketDeEscucha;

void planificacionSJF(bool desalojo){


	pthread_create(&hiloEscuchaConsola, NULL, (void *) lanzarConsola, NULL);

	pthread_create(&hiloEscuchaESI, NULL, (void *) escucharNuevosESIS, NULL);

	while (queue_size(colaListos) == 0){

		if (queue_size(colaListos) > 0){
			break;
		}

	}

	log_info(logPlanificador, "Comienza planificacion SJF");

	log_info(logPlanificador, "Conectando servidor");

	log_info(logPlanificador,"Arraca SJF");

	estimarTiempos();

	log_info(logPlanificador, "Tiempos de los ESI listos estimados");

	armarColaListos();

	log_info(logPlanificador, "Cola de ESI armada");


	while ( !queue_is_empty(colaListos)){

		bool finalizar = false;

		bool bloquear = false;

		bool permiso = true;

		ESI * nuevo = queue_pop(colaListos);

		uint32_t operacion;
		uint32_t tamanioRecurso;
		char * recursoPedido;


		while(!finalizar && !bloquear && permiso){

				send(nuevo->id,&CONTINUAR,sizeof(uint32_t),0);
				int respuesta1 = recv(socketCoordinador, &operacion, sizeof(operacion), 0);
				int respuesta2 = recv(socketCoordinador, &tamanioRecurso, sizeof(uint32_t), 0);
				recursoPedido = malloc(sizeof(tamanioRecurso));
				int respuesta3 = recv(socketCoordinador, recursoPedido, sizeof(char)*tamanioRecurso,0);


				if(respuesta1 < 0 || respuesta2 < 0 || respuesta3 < 0){
					log_info(logPlanificador, "conexion con el coordinador rota");
					exit(-1);
				} else {
					nuevo->recursoPedido = recursoPedido;
					nuevo->proximaOperacion = operacion;

				}

				permiso = validarPedido(nuevo->recursoPedido,nuevo);

				if(permiso){

					char * recursoAUsar = nuevo->recursoPedido;

					if(!recursoEnLista(nuevo->recursoPedido, nuevo->recursosAsignado)){

						list_add(listaRecursos, nuevo->recursosAsignado);

					}

					bloquearRecurso(recursoAUsar);

					log_info(logPlanificador, " ESI de clave %s entra al planificador", nuevo->id );

					send(socketCoordinador, &CONTINUAR, sizeof(uint32_t),0);

					log_info(logPlanificador, " ejecuta una sentencia ");

					nuevo -> rafagaAnterior = nuevo-> rafagaAnterior +1;
					nuevo -> rafagasRealizadas = nuevo -> rafagasRealizadas +1;

					log_info(logPlanificador, "rafagas realizadas del esi %s son %d", nuevo-> id, nuevo->rafagasRealizadas);

					int respuesta ;
					recv(nuevo->id, &respuesta, sizeof(int),0);

					if (respuesta != CONTINUAR)
					{
						finalizar = true;
					} else if (nuevo->id == claveParaBloquearESI)
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
			log_info(logPlanificador, " ESI de clave %s en finalizados!", nuevo->id);
			liberarRecursos(nuevo);
			armarColaListos();

		} else if( bloquear ){ // este caso sería para bloqueados por usuario. No se libera clave acá

			nuevo->bloqueadoPorUsuario = true;
			bloquearRecurso(claveParaBloquearRecurso);
			bloquearESI(claveParaBloquearRecurso,nuevo);
			log_info(logPlanificador, " ESI de clave %s en bloqueados para recurso %s", nuevo->id, claveParaBloquearESI);

		}

		ESI_destroy(nuevo);

	}

	planificacionSJFTerminada = true;


}

void estimarTiempos(){

	log_info(logPlanificador,"Estimacion de tiempos empezada \n");

	listaListos = list_map(listaListos, (void *)estimarProximaRafaga);

	log_info(logPlanificador,"terminada la estimacion \n");

}



void armarColaListos(){

	log_info(logPlanificador,"armando cola de listos \n");

	ESI * ESIMasRapido;

	while (0 < list_size(listaListos)){

		int i = 1;
		ESIMasRapido = list_get(listaListos, 0);

		while (i < list_size(listaListos)){

			ESI * ESIAuxiliar = list_get(listaListos, i);
			if (ESIMasRapido->estimacionSiguiente > ESIAuxiliar->estimacionSiguiente){

				ESIMasRapido = ESIAuxiliar;

			}

			i++;
		}

		log_info(logPlanificador,"ESI numero %d es de estimacion: %d \n", i-1, ESIMasRapido->estimacionSiguiente);

		claveActual = ESIMasRapido->id;
		list_remove_by_condition(listaListos, (void *) compararClaves);
		queue_push(colaListos,ESIMasRapido);
	}

	log_info(logPlanificador,"cola armada \n");


}


void escucharPedidos(int conexion){


	uint32_t a = 1234;
	send(conexion, &a, sizeof(uint32_t),  0); // Envio al ESI lo que se eligio en consola
	uint32_t respuesta;
	recv(conexion, &respuesta , sizeof (uint32_t), 0);
	printf("Me respondio: %d\n", respuesta);




}



void liberarBloqueados(){
/*
	int i = 0;
	while(i < list_size(listaBloqueados)){

		if(!recursoGenericoEnUso){ //deberia meter a todos en listos porque el recurso esta libre siempre basicamente

			list_remove(listaBloqueados, i);
			list_add(listaListos, list_remove(listaBloqueados, i));
			log_info(logPlanificador, " se libero un ESI bloqueado por recurso");

		}

		i++;
	}

	armarColaListos(); //rearmo la cola de listos cuando entra el nuevo ESI.
*/

}
