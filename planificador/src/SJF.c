#include "SJF.h"

int socketDeEscucha;

void planificacionSJF(){

	log_info(logPlanificador, "Comienza planificacion SJF");

	log_info(logPlanificador, "Conectando servidor");

	pthread_create(&hiloEscuchaConsola, NULL, (void *) lanzarConsola, NULL);

	log_info(logPlanificador,"Arraca SJF");

	estimarTiempos();

	log_info(logPlanificador, "Tiempos de los ESI listos estimados");

	armarColaListos();

	log_info(logPlanificador, "Cola de ESI armada");


	while ( !queue_is_empty(colaListos)){

		bool finalizar = false;

		bool bloquear = false;

		ESI * nuevo = queue_pop(colaListos);

		int socketESI = escucharCliente(logPlanificador, socketDeEscucha, 1);

		char * recursoAUsar = nuevo->recursoPedido;

		bloquearRecurso(recursoAUsar);

		log_info(logPlanificador, "Se conecto un ESI!");

		pthread_create(&hiloEscuchaESI,NULL, (void *) escucharPedidos, &socketESI);

		log_info(logPlanificador, " ESI de clave %s entra al planificador", nuevo->id );

		while(!finalizar && !bloquear){

			log_info(logPlanificador, " ejecuta una sentencia ");
			//send a ESI el permiso para ejecutar
			send(socketESI,(void *)CONTINUAR,sizeof(int),0 );
			nuevo -> rafagaAnterior = nuevo-> rafagaAnterior +1;
			nuevo -> rafagasRealizadas = nuevo -> rafagasRealizadas +1;
			log_info(logPlanificador, "rafagas realizadas del esi %s son %d", nuevo-> id, nuevo->rafagasRealizadas);

			int respuesta ;
			recv(socketESI, &respuesta, sizeof(int),0);

			if (respuesta != CONTINUAR)
			{
				finalizar = true;
			} else if (string_equals_ignore_case(nuevo->id, claveParaBloquearESI))
			{
				send(socketESI,(void *)FINALIZAR,sizeof(int),0 );
				bloquear = true;

			}

		}

		log_info(logPlanificador,"finalizada su rafaga");

		if( finalizar ){ //aca con el mensaje del ESI, determino si se bloquea o se finaliza

			list_add ( listaFinalizados, nuevo);
			log_info(logPlanificador, " ESI de clave %s en finalizados!", nuevo->id);
			//todo liberar clave

		} else if( bloquear ){ // este caso sería para bloqueados por usuario. No se libera clave acá

			nuevo->bloqueadoPorUsuario = true;
			t_ESIBloqueado * nuevoBloqueado = malloc(sizeof(t_ESIBloqueado));
			nuevoBloqueado -> bloqueado = nuevo;
			nuevoBloqueado -> claveRecurso = claveParaBloquearRecurso;


			queue_push(colaBloqueados, nuevoBloqueado);
			free(nuevoBloqueado);

			log_info(logPlanificador, " ESI de clave %s en bloqueados para recurso %s", nuevo->id, claveParaBloquearESI);

		}

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

void liberarRecursos(int recursoID){


	//todo acorde a lo que se defina de recursos, acá busco por ID y libero. Por ahora tengo uno solo

	recursoGenericoEnUso = false;


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


void planificacionSJFConDesalojo(){

	socketDeEscucha = conectarComoServidor(logPlanificador, ip, puerto, 1);
//	int socketESI = escucharCliente(logPlanificador, *(int*) socketDeEscucha, 1);
	pthread_create(&hiloEscuchaConsola,NULL, (void *) escucharPedidos, NULL);

	log_info(logPlanificador,"Arraca SJF con desalojo");

	planificacionSJFTerminada = false;

	while(!planificacionSJFTerminada){ // Aca la idea sería que el hilo que escucha los pedidos de la consola, me tire una varaible global para que termine el while y replanifique por desalojo

		planificacionSJF(); // planificacion SJF comun si no se desbloquean nuevos procesos

	}

	if(!planificacionSJFTerminada){ // si la planificacion no terminó, acá vuelve a entrar a SJF con desalojo, para replanificar

		planificacionSJFConDesalojo();

	}


}
