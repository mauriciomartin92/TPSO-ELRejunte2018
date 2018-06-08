#include "HRRN.h"
#include <pthread.h>


bool planificacionHRRNTerminada = false;

void
planificacionHRRN ()
{


  liberarProcesosBloqueados ();
  pthread_create (&hiloEscuchaConsola, NULL, (void *) escucharPedidos, NULL);

  log_info (logPlanificador, "Arraca HRRN");

  estimarYCalcularTiempos ();

  log_info (logPlanificador, "Todos los tiempos listos");

  armarCola ();

  log_info (logPlanificador, "Cola armada");


  while (!queue_is_empty (colaListos))
    {

      bool finalizar = false;

      ESI* nuevoESI = queue_pop (colaListos);

      log_info (logPlanificador, " ESI de clave %s entra al planificador",
		nuevoESI->id);



      while (!finalizar)
	{

	  recursoGenericoEnUso = true;
	  log_info (logPlanificador, " ejecuta una sentencia ");
	  //send a ESI el permiso para ejecutar
	  nuevoESI->rafagaAnterior = nuevoESI->rafagaAnterior + 1;
	  nuevoESI->rafagasRealizadas = nuevoESI->rafagasRealizadas + 1;
	  log_info (logPlanificador, "rafagas realizadas del esi %s son %d",
		    nuevoESI->id, nuevoESI->rafagasRealizadas);


	  if (nuevoESI->rafagasRealizadas == 3)	//de nuevo, harcodeo para testear que funcione. acá debería llegar el mensaje de terminacion
	    {
	      finalizar = true;
	    }

	}

      log_info (logPlanificador, "finalizada su rafaga");
      liberarRecursos (1);	// acá debería liberarse el recurso que usó el ESI

      if (1 == 2)
	{			//aca con el mensaje del ESI, determino si se bloquea o se finaliza

	  list_add (listaFinalizados, nuevoESI);
	  log_info (logPlanificador, " ESI de clave %s en finalizados!",
		    nuevoESI->id);

	}
      else if (1 == 2)
	{			// acá bloqueo por recurso

	  nuevoESI->rafagasRealizadas = 0;
	  nuevoESI->bloqueadoPorRecurso = true;
	  list_add (listaBloqueados, nuevoESI);
	  log_info (logPlanificador, " ESI de clave %s en bloqueados !",
		    nuevoESI->id);


	}
      else
	{			// este caso sería para bloqueados por usuario

	  nuevoESI->bloqueadoPorUsuario = true;
	  list_add (listaBloqueados, nuevoESI);
	  log_info (logPlanificador, " ESI de clave %s en bloqueados!",
		    nuevoESI->id);

	}

      liberarProcesosBloqueados ();

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


void
liberarProcesosBloqueados ()
{

  int i = 0;
  while (i < list_size (listaBloqueados))
    {

      if (!recursoGenericoEnUso)
	{			//deberia meter a todos en listos porque el recurso esta libre siempre basicamente

	  list_remove (listaBloqueados, i);
	  list_add (listaListos, list_remove (listaBloqueados, i));
	  log_info (logPlanificador,
		    " se libero un ESI bloqueado por recurso");

	}

      i++;
    }

  armarCola ();			//rearmo la cola de listos cuando entra el nuevo ESI.


}



void
planificacionHRRNConDesalojo ()
{


  liberarProcesosBloqueados ();
  pthread_create (&hiloEscuchaConsola, NULL, (void *) escucharPedidos, NULL);

  log_info (logPlanificador, "Arraca SJF con desalojo");

  planificacionHRRNTerminada = false;

  while (1 && !planificacionHRRNTerminada)
    {

      planificacionHRRN ();
    }

  if (!planificacionHRRNTerminada)
    {

      planificacionHRRNConDesalojo ();

    }
}

