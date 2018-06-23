#ifndef SJF_H_
#define SJF_H_

#include <pthread.h>

#include "planificador.h"

void planificacionSJF();
void planificacionSJFConDesalojo();
void estimarTiempos();
void armarColaListos();
void liberarBloqueados();

bool planificacionSJFTerminada;


#endif /* SJF_H_ */
