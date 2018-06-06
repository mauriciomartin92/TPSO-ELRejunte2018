#ifndef CONSOLA_H_
#define CONSOLA_H_


#include "Planificador.h"
#include <readline/readline.h>


/*
 * Acá había pensado que se podría usar la opción para que funcione para su contraparte también
 * Por ejemplo, se selecciona BLOQUEAR, y si el ESI ya está bloqueado, se lo desbloquea.
 * Peeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeero por ahora lo dejo así--> a discutir.
 *
 */

void lanzarConsola();


char * PAUSEAR_PLANIFICACION= "pausear_planificacion";
char* REANUDAR_PLANIFICACION = "reanudar_planificacion";
char* BLOQUEAR_ESI = "bloquear_esi";
char* DESBLOQUEAR_ESI = "desbloquear_esi";
char* LISTAR_POR_RECURSO = "listar_por_recurso";
char* KILL_ESI = "kill_esi";
char* STATUS_ESI = "status_esi";
char* COMPROBAR_DEADLOCK = "comprobar_deadlock";



/*
 *
 *
 * En los sockets iria algo asi para la comunicacion de consola con planificador si se decide que sean procesos separados. No me parece mala idea
 *
 *
 *

enum ordenes{

	PAUSEAR_ESI = 0,
	REANUDAR_ESI,
	BLOQUEAR_ESI,
	DESBLOQUEAR_ESI,
	LISTAR_RECURSO,
	KILL_ESI,
	STATUS_ESI,
	DEADLOCK

};

*/







#endif /* CONSOLA_H_ */
