#include "SJF.h"

void planificacionSJF(){

	liberarBloqueados();

	socketDeEscucha = conectarComoServidor(logPlanificador, ipPropia, puertoPropio, backlog);

	int socketESI = escucharCliente(logPlanificador, *(int*) socketDeEscucha, backlog);

	pthread_create(&hiloEscuchaConsola,NULL, (void *) escucharPedidos, &socketESI);
	pthread_create(&hiloEscuchaESI, NULL, (void *) lanzarConsola, NULL);

	log_info(logPlanificador,"Arraca SJF");

	estimarTiempos();

	log_info(logPlanificador, "Tiempos de los ESI listos estimados");

	armarColaListos();

	log_info(logPlanificador, "Cola de ESI armada");


	while ( !queue_is_empty(colaListos)){

		bool finalizar = false;

		ESI * nuevo = queue_pop(colaListos);

		log_info(logPlanificador, " ESI de clave %s entra al planificador", nuevo->id );

		int l = 0;

		while(!finalizar){


			recursoGenericoEnUso = true;
			log_info(logPlanificador, " ejecuta una sentencia ");
			//send a ESI el permiso para ejecutar

			nuevo -> rafagaAnterior = nuevo-> rafagaAnterior +1;
			nuevo -> rafagasRealizadas = nuevo -> rafagasRealizadas +1;
			log_info(logPlanificador, "rafagas realizadas del esi %s son %d", nuevo-> id, nuevo->rafagasRealizadas);


			if (l == 3) //harcodeo para testear que funcione. acá debería llegar el mensaje de terminacion
			{
				finalizar = true;
			}

			l++;

		}

		log_info(logPlanificador,"finalizada su rafaga");
		liberarRecursos(1); // acá debería liberarse el recurso que usó el ESI

		if( 1 == 1){ //aca con el mensaje del ESI, determino si se bloquea o se finaliza

			list_add ( listaFinalizados, nuevo);
			log_info(logPlanificador, " ESI de clave %s en finalizados!", nuevo->id);

		} else if (2 == 2){ // acá bloqueo por recurso si terminó su rafaga (bloqueado por recurso)

			nuevo->rafagasRealizadas = 0;
			nuevo->bloqueadoPorRecurso = true;
			list_add(listaBloqueados, nuevo);
			log_info(logPlanificador, " ESI de clave %s en bloqueados !", nuevo->id);


		} else { // este caso sería para bloqueados por usuario o desalojados, como no me termina el proceso, sigo teniendo en cuenta las rafagas que realizó en su pasada.

			nuevo->bloqueadoPorUsuario = true;
			list_add(listaBloqueados, nuevo);
			log_info(logPlanificador, " ESI de clave %s en bloqueados!", nuevo->id);

		}

		liberarBloqueados();
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


	int a= 1234;
	send(conexion, &a, sizeof(uint32_t),  0); // Envio al ESI lo que se eligio en consola
	int respuesta;
	recv(conexion, &respuesta , sizeof (uint32_t), 0);
	log_info(logPlanificador, " entra al hilo ");

}

void liberarRecursos(int recursoID){


	//todo acorde a lo que se defina de recursos, acá busco por ID y libero. Por ahora tengo uno solo

	recursoGenericoEnUso = false;


}


void liberarBloqueados(){

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


}


void planificacionSJFConDesalojo(){


	liberarBloqueados();
	int socketESI = escucharCliente(logPlanificador, *(int*) socketDeEscucha, backlog);
	pthread_create(&hiloEscuchaConsola,NULL, (void *) escucharPedidos, NULL);

	log_info(logPlanificador,"Arraca SJF con desalojo");

	planificacionSJFTerminada = false;

	while(1 && !planificacionSJFTerminada){ // Aca la idea sería que el hilo que escucha los pedidos de la consola, me tire una varaible global para que termine el while y replanifique por desalojo

		planificacionSJF(); // planificacion SJF comun si no se desbloquean nuevos procesos

	}

	if(!planificacionSJFTerminada){ // si la planificacion no terminó, acá vuelve a entrar a SJF con desalojo, para replanificar

		planificacionSJFConDesalojo();

	}


}
