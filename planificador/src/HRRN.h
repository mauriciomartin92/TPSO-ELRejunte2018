#ifndef HRRN_H_
#define HRRN_H_


#include <pthread.h>
#include "planificador.h"

void planficacionHRRN();
void planificacionHRRNConDesalojo();
void estimarYCalcularTiempos();
float calcularTiempoEspera (float espera, int estimacionSiguiente);
void armarCola ();
void sumarTiemposEspera ();
void liberarProcesosBloqueados();


bool planificacionHRRNTerminada;
#endif /* HRRN_H_ */
