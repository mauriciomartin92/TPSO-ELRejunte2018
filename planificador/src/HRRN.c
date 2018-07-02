#include "HRRN.h"
#include <pthread.h>


bool planificacionHRRNTerminada = false;

void
planificacionHRRN ()
{


  pthread_create (&hiloEscuchaConsola, NULL, (void *) escucharPedidos, NULL);

  log_info (logPlanificador, "Arraca HRRN");


  estimarYCalcularTiempos ();

  log_info (logPlanificador, "Todos los tiempos listos");

  armarCola ();

  log_info (logPlanificador, "Cola armada");


  while (!queue_is_empty (colaListos)) //todo implementar los hilos
    {

      bool finalizar = false;
      bool bloquear = false;

      ESI* nuevoESI = queue_pop (colaListos);

      log_info(logPlanificador, "Conectando servidor");

  /*    socketDeEscucha = conectarComoServidor(logPlanificador, ip,puerto, 1);

      int socketESI = escucharCliente(logPlanificador, socketDeEscucha, 1);
*/
      log_info(logPlanificador, "Se conecto un ESI!");

      log_info (logPlanificador, " ESI de clave %s entra al planificador",
		nuevoESI->id);



      while (!finalizar && !bloquear)
	{

	  log_info (logPlanificador, " ejecuta una sentencia ");
//	  send(socketESI,(void *)CONTINUAR,sizeof(int),0 );
	  nuevoESI->rafagaAnterior = nuevoESI->rafagaAnterior + 1;
	  nuevoESI->rafagasRealizadas = nuevoESI->rafagasRealizadas + 1;

	  log_info (logPlanificador, "rafagas realizadas del esi %s son %d",
		    nuevoESI->id, nuevoESI->rafagasRealizadas);

	  int respuesta = 0 ;

//	  recv(socketESI, &respuesta, sizeof(int),0);

	  if (respuesta != CONTINUAR)	//de nuevo, harcodeo para testear que funcione. acá debería llegar el mensaje de terminacion
	    {
	      finalizar = true;
	    } else if(nuevoESI->id ==claveParaBloquearESI)
	    {
	//    	send(socketESI,(void *)FINALIZAR,sizeof(int),0 );
	    	bloquear = true;
	    }

	}

      log_info (logPlanificador, "finalizada su rafaga");

      if (finalizar)
	{			//aca con el mensaje del ESI, determino si se bloquea o se finaliza

		  list_add (listaFinalizados, nuevoESI);
		  log_info (logPlanificador, " ESI de clave %s en finalizados!",
				nuevoESI->id);

	}
      else if (bloquear)
	{			// acá bloqueo usuario

    	  nuevoESI->bloqueadoPorUsuario = true;
    	  t_ESIBloqueado * nuevoBloqueado = malloc(sizeof(t_ESIBloqueado));
    	  nuevoBloqueado -> bloqueado = nuevoESI;
    	  nuevoBloqueado -> claveRecurso = claveParaBloquearRecurso;
    	  queue_push(colaBloqueados, nuevoBloqueado);
    	  free(nuevoBloqueado);

    	  log_info(logPlanificador, " ESI de clave %s en bloqueados para recurso %s", nuevoESI->id, claveParaBloquearESI);


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


  while (i <= list_size (listaListos))
    {

      log_info (logPlanificador, "Entra ESI clave %d", nuevo->id);

      nuevo = list_remove (listaListos, i);

      estimarProximaRafaga (nuevo);

      nuevo->tiempoEspera =
	calcularTiempoEspera (nuevo->tiempoEspera,
			      nuevo->estimacionSiguiente);

      log_info (logPlanificador, "un tiempo calculado");

    }


}


void
armarCola ()
{

  log_info (logPlanificador, "armando cola de listos \n");

  ESI *ESIPrioridad;

  while (0 < list_size (listaListos))
    {

      int i = 1;
      ESIPrioridad = list_get (listaListos, 0);

      while (i <= list_size (listaListos))
	{

	  ESI *ESIAuxiliar = list_get (listaListos, i);
	  if (ESIPrioridad->tiempoEspera < ESIAuxiliar->tiempoEspera)
	    {

	      ESIPrioridad = ESIAuxiliar;

	    }

	  i++;
	}

      log_info (logPlanificador,
		"ESI numero %d, de clave %s, es de tiempo de respuesta: %.6f, y su proxima rafaga se estima en %d ",
		i - 1, ESIPrioridad->id, ESIPrioridad->tiempoEspera,
		ESIPrioridad->estimacionSiguiente);

      claveActual = ESIPrioridad->id;
      list_remove_by_condition (listaListos, (void *) compararClaves);
      queue_push (colaListos, ESIPrioridad);
    }

  log_info (logPlanificador, "cola armada \n");

}



float
calcularTiempoEspera (float espera, int estimacionSiguiente)
{

  log_info (logPlanificador, "calculando..");
  return (espera + estimacionSiguiente) / estimacionSiguiente;

}

/*
void
liberarProcesosBloqueados ()
{

  int i = 0;
  while (i < queue_size (colaBloqueados))
    {

      if (!recursoGenericoEnUso)
	{			//deberia meter a todos en listos porque el recurso esta libre siempre basicamente

	  queue_pop (colaBloqueados);
	  queue_push (colaBloqueados, list_remove (colaBloqueados, i));
	  log_info (logPlanificador,
		    " se libero un ESI bloqueado por recurso");

	}

      i++;
    }

  armarCola ();			//rearmo la cola de listos cuando entra el nuevo ESI.


}
*/


void
planificacionHRRNConDesalojo ()
{



  pthread_create (&hiloEscuchaConsola, NULL, (void *) escucharPedidos, NULL);

  log_info (logPlanificador, "Arraca SJF con desalojo");

  planificacionHRRNTerminada = false;

  while (!planificacionHRRNTerminada) //todo acá voy a tener que copypastear lo de arriba, no permitiría el desalojo esto
    {

      planificacionHRRN ();
    }

  if (!planificacionHRRNTerminada)
    {

      planificacionHRRNConDesalojo ();

    }
}

