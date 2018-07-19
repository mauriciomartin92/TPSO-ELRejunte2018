#include "planificador.h"

bool planificacionHRRNTerminada = false;

void
planificacionHRRN (bool desalojo)
{

  pthread_create (&hiloEscuchaConsola, NULL, (void *) lanzarConsola, NULL);

  pthread_create(&hiloEscuchaESI, NULL, (void *) escucharNuevosESIS, NULL);

  while (1)
    {

  while (queue_size(colaListos) == 0){

	if (queue_size(colaListos) > 0){
		break;
	}
  }

  log_info (logPlanificador, "Arraca HRRN");

	bool finalizar = false;

	bool bloquear = false;

	bool permiso = true;

	bool desalojar = false;

	uint32_t operacion;
	uint32_t tamanioRecurso;
	char * recursoPedido;

	ESI* nuevoESI = queue_pop (colaListos);

  while (!finalizar && !bloquear && permiso && !desalojar && !matarESI)
{

  while(pausearPlanificacion){}

  pthread_mutex_lock(&mutexComunicacion);

	log_info(logPlanificador, "empieza comunicacion");


  send(nuevoESI->id,&CONTINUAR,sizeof(uint32_t),0);
  int respuesta1 = recv(socketCoordinador, &operacion, sizeof(operacion), 0);
  int respuesta2 = recv(socketCoordinador, &tamanioRecurso, sizeof(uint32_t), 0);
  recursoPedido = malloc(sizeof(char)*tamanioRecurso);
  int respuesta3 = recv(socketCoordinador, recursoPedido, sizeof(char)*tamanioRecurso,0);

	log_info(logPlanificador, "recibo datos suficientes para corroborar permiso");


  if(respuesta1 <= 0 || respuesta2 <= 0 || respuesta3 <= 0){
	  log_info(logPlanificador, "conexion con el coordinador rota");
	  liberarGlobales();
	  exit(-1);
  } else {
	  free(nuevoESI->recursoPedido);
	  log_info(logPlanificador, "se carga recurso pedido : %s", recursoPedido);
	  nuevoESI->recursoPedido = string_new();
	  string_append(&nuevoESI->recursoPedido, recursoPedido);
	  nuevoESI->proximaOperacion = operacion;
  }

  permiso = validarPedido(nuevoESI->recursoPedido,nuevoESI);

  if(permiso){


	  nuevoESI->tiempoEspera = 0;

	  log_info(logPlanificador, "ESI tiene permiso de ejecucion");

	  if(operacion == 2){

		  char * valorRecurso;
		  uint32_t tamValor;

		  log_info(logPlanificador, "recibo mas datos para ejecucion");

		  int resp = recv (socketCoordinador, &tamValor,sizeof(uint32_t),0);
		  valorRecurso=malloc(sizeof(char)*tamValor);
		  int resp2 =recv (socketCoordinador,valorRecurso,sizeof(char)*tamValor,0);

		  if(resp <= 0 || resp2 <= 0){

			  log_info(logPlanificador, "conexion con el coordinador rota");
			  liberarGlobales();
			  exit(-1);

		  } else {
			  log_info(logPlanificador, "cargo valor obtenidos");
			  cargarValor(recursoPedido,valorRecurso);
		  }


	  }
	  pthread_mutex_unlock(&mutexComunicacion);

	  log_info(logPlanificador, "terminada comunicacion");

	  if(nuevoESI->proximaOperacion == 1 && !recursoEnLista(nuevoESI)){

		  log_info(logPlanificador, "el recurso se añade a asignados");
		  list_add(nuevoESI->recursosAsignado, recursoPedido);

	  }

	  log_info(logPlanificador, "se bloquea recurso a usar");

	  bloquearRecurso(recursoPedido);

	  nuevoESI->recienLlegado = false;
	  nuevoESI->recienDesbloqueadoPorRecurso = false;

	  log_info(logPlanificador, " ESI de clave %d entra al planificador", nuevoESI->id );

	  pthread_mutex_lock(&mutexComunicacion);

	  send(socketCoordinador, &CONTINUAR, sizeof(uint32_t),0);

	  log_info(logPlanificador, " ejecuta una sentencia ");

	  nuevoESI -> rafagaAnterior = nuevoESI-> rafagaAnterior +1;
	  nuevoESI -> rafagasRealizadas = nuevoESI -> rafagasRealizadas +1;

	  pthread_mutex_lock(&mutexColaListos);

	  log_info(logPlanificador, "se procede a aumentar tiempos de espera");

	  aumentarEspera();

	  pthread_mutex_unlock(&mutexColaListos);

	  log_info(logPlanificador, "rafagas realizadas del esi %d son %d", nuevoESI-> id, nuevoESI->rafagasRealizadas);

	  uint32_t respuesta ;
	  int conexion = recv(nuevoESI->id, &respuesta, sizeof(uint32_t),0);

	  pthread_mutex_unlock(&mutexComunicacion);

	  if(conexion <= 0){
		  log_info(logPlanificador, "se rompio la conexion");
		  liberarGlobales();
		  exit(-1);
	  }
	  if (respuesta != CONTINUAR)
	  {
		  log_info(logPlanificador, "el ESI quiere finalizar");
		  finalizar = true;

	  } else if(desalojo && queue_size(colaListos) > 0) // si hay desalojo activo
	  {
		  log_info(logPlanificador, "Chequeo si hay que desalojar");

		  pthread_mutex_lock(&mutexColaListos);
		  ESI* auxiliar = queue_peek(colaListos);

		  if(auxiliar->recienLlegado) //chequeo si el proximo en cola es un recien llegado
		  {
			  if(auxiliar->tiempoEspera < (nuevoESI->tiempoEspera - nuevoESI->rafagasRealizadas)) // y si su estimacion siguiente es menor que la del que esta en ejecucion menos lo que ya hizo
			  {
				  log_info(logPlanificador, "se genera desalojo");

				  desalojar = true; //se va a desalojar
			  } else
			  {
				  log_info(logPlanificador, "no hay necesidad de desalojar, se actualiza cola");

				  limpiarRecienLlegados(); // Si no es menor la estimacion, los recien llegados ya no tendrian validez, los actualizo
			  }
		  }
		  pthread_mutex_unlock(&mutexColaListos);

	  }

	  if (nuevoESI->id == claveParaBloquearESI)
	  {
		  log_info(logPlanificador, "se pide bloqueo del esi");

		  bloquear = true;
	  }



  } else {

	  pthread_mutex_unlock(&mutexComunicacion);

	  log_info(logPlanificador, "El ESI no tiene permiso de ejecucion");


	  bloquearESI(nuevoESI->recursoPedido, nuevoESI);

  }

}

      log_info (logPlanificador, "finalizada su rafaga");

      if(matarESI){

    	  log_info(logPlanificador, "ESI de clave %d fue matado por consola", nuevoESI->id);
		  log_info(logPlanificador, "le comunico al coordinador");
    	  pthread_mutex_lock(&mutexComunicacion);
    	  send(socketCoordinador,&FINALIZAR,sizeof(uint32_t),0);
    	  send(socketCoordinador,&claveActual,sizeof(claveActual),0); //todo decirle a gabi q es int
    	  pthread_mutex_unlock(&mutexComunicacion);
		  log_info(logPlanificador, "terminada comunicacion");
		  log_info(logPlanificador, " se liberan sus recursos y se lo destruye ");

    	  liberarRecursos(nuevoESI);
    	  ESI_destroy(nuevoESI);

    	  pthread_mutex_lock(&mutexAsesino);

    	  matarESI = false;

    	  pthread_mutex_unlock(&mutexAsesino);
      } else if (finalizar)
	{			//aca con el mensaje del ESI, determino si se bloquea o se finaliza

		  list_add (listaFinalizados, nuevoESI);
		  log_info (logPlanificador, " ESI de clave %s en finalizados!",
				nuevoESI->id);
		  liberarRecursos(nuevoESI);
		  if(bloquearESIActual) bloquearESIActual = false;


	}
      else if (bloquear)
	{			// acá bloqueo usuario

    	  nuevoESI->bloqueadoPorUsuario = true;
    	  bloquearRecurso(claveParaBloquearRecurso);
    	  bloquearESI(claveParaBloquearRecurso,nuevoESI);
    	  bloquearESIActual = false;

    	  log_info(logPlanificador, " ESI de clave %s en bloqueados para recurso %s", nuevoESI->id, claveParaBloquearESI);


	}
      else if (desalojar)
      {
    	  list_add(listaListos,nuevoESI);
    	  log_info(logPlanificador," ESI de clave %d desalojado", nuevoESI->id);
    	  armarCola();

      }


    }
  planificacionHRRNTerminada = true;

}



void
estimarYCalcularTiempos ()
{


  log_info (logPlanificador,
	    "Empieza la estimación de ráfagas y tiempos de respuesta");
  int i = 0;

  ESI *nuevo;

  while (i < list_size (listaListos))
    {
      nuevo = list_get (listaListos, i);

      log_info (logPlanificador, "Entra ESI clave %d", nuevo->id);


      estimarProximaRafaga (nuevo);

      nuevo->tiempoRespuesta =
	calcularTiempoEspera (nuevo->tiempoEspera,
			      nuevo->estimacionSiguiente);

      log_info (logPlanificador, "un tiempo calculado: %.6f", nuevo->tiempoRespuesta);

      i++;

    }


}


void
armarCola ()
{

	  estimarYCalcularTiempos();

	  log_info(logPlanificador, "Tiempos de los ESI listos estimados");

	  log_info (logPlanificador, "armando cola de listos ");

	  ESI *ESIPrioridad;

	  while (0 < list_size (listaListos))
		{
		  int i = 1;
		  ESIPrioridad = list_get (listaListos, 0);

		  while (i < list_size (listaListos) && list_size(listaListos) > 0)
		{

		  ESI *ESIAuxiliar = list_get (listaListos, i);
		  if (ESIPrioridad->tiempoRespuesta < ESIAuxiliar->tiempoRespuesta)
			{

			  ESIPrioridad = ESIAuxiliar;

			} else if ( ESIPrioridad-> tiempoRespuesta == ESIAuxiliar -> tiempoRespuesta){

				log_info(logPlanificador, "igualdad en tiempos de respuesta");

				if( !ESIAuxiliar->recienLlegado && ESIPrioridad->recienLlegado){ //si no es recien llegado, tiene prioridad porque ya estaba en disco

					log_info(logPlanificador, "gana esi nuevo por recien llegado");

					ESIPrioridad = ESIAuxiliar;

				} else if( !ESIAuxiliar->recienLlegado && !ESIPrioridad->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIPrioridad->recienDesbloqueadoPorRecurso ){ // si se da que ninguno de los dos recien fue creado, me fijo si alguno se desbloqueo recien de un recurso

					log_info(logPlanificador, "gana esi nuevo por recien desbloqueado de recurso");

					ESIPrioridad = ESIAuxiliar;

				} else if ( ESIAuxiliar->recienLlegado && ESIPrioridad->recienLlegado && ESIAuxiliar->recienDesbloqueadoPorRecurso && !ESIPrioridad->recienDesbloqueadoPorRecurso ){ //si los dos recien llegan, me fijo si el auxiliar recien llego de desbloquearse

					log_info(logPlanificador, "gana esi nuevo por ser recien desbloqueado por recurso");

					ESIPrioridad = ESIAuxiliar;

				}
			}

		  i++;
		}

		  log_info (logPlanificador,
			"ESI entra en cola y es de tiempo de respuesta: %.6f, y su proxima rafaga se estima en %d ",
			ESIPrioridad->tiempoRespuesta,
			ESIPrioridad->estimacionSiguiente);

		  claveActual = ESIPrioridad->id;
		  ESIPrioridad->bloqueadoPorUsuario = false;
		  queue_push (colaListos, ESIPrioridad);
		  log_info(logPlanificador, "esi en cola. Eliminandolo de la lista de listos");

		  list_remove_by_condition (listaListos, (void *) compararClaves);

		}

	  log_info (logPlanificador, "cola armada \n");

}



float
calcularTiempoEspera (float espera, int estimacionSiguiente)
{

  log_info (logPlanificador, "calculando tiempo espera..");
  return (espera + estimacionSiguiente) / estimacionSiguiente;

}

void aumentarEspera(){

	t_queue * aux = queue_create();

	while(!queue_is_empty(colaListos)){

		ESI * nuevo= queue_pop(colaListos);
		log_info(logPlanificador, "aumentando tiempo espera para : %d", nuevo->id);
		nuevo->tiempoEspera = nuevo->tiempoEspera +1;
		log_info(logPlanificador, "queda : %d", nuevo->tiempoEspera);

		queue_push(aux, nuevo);
	}

	queue_destroy(colaListos);
	colaListos = aux;

}
