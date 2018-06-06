#include "consola.h"


void lanzarConsola(){



	char* linea;

	while(1){

		printf("¿Que operacion desea hacer?\n");
		printf("Pausear_planificacion \n");
		printf("Reanudar_planificacion \n");
		printf("Bloquear_ESI \n");
		printf("Desbloquear_ESI \n");
		printf("Listar_por_recurso \n");
		printf("Kill_ESI \n");
		printf("Status_ESI \n");
		printf("Comprobar_Deadlock \n");
		printf("Salir");
		printf("Ingrese el nombre de la opcion, incluyendo el guion bajo");
		linea = readline(">");

		if (string_equals_ignore_case(linea, PAUSEAR_PLANIFICACION))  //Hermosa cadena de if que se viene
		{
			//todo funcion que pausee
			// Conectar con planificador e indicarle que el ESI se pausea pero NO se bloquea, solo para temporalmente la planificacion
			break;
		}
		else if (string_equals_ignore_case(linea,REANUDAR_PLANIFICACION))
		{
			//todo funcion que reanude
			//reanuda lo pauseado -> podrian ser la misma opcion con pausear porque la comprobacion se hace igual
			break;
		}
		else if (string_equals_ignore_case(linea, BLOQUEAR_ESI))
		{
			//todo funcion que bloquee
			linea = readline("CLAVE:");
			//mandar clave al planificador
			// Manda al ESI a la lista de bloqueados si esta en ejecucion o listo para ejecurar
			break;
		}
		else if (string_equals_ignore_case(linea,DESBLOQUEAR_ESI))
		{
			//todo funcion que desbloquee
			linea = readline("CLAVE:");
			// Vuelve a meter el ESI a la cola de listos -> podria ser una sola opcion, como en mi comentario anterior
			break;
		}
		else if (string_equals_ignore_case(linea, LISTAR_POR_RECURSO)){

			//todo funcion que liste
			linea = readline("RECURSO:");
			//Aca agarra la clave del recurso a usar y muestra por pantalla los recursos que estan esperando usarlo
			break;
		}
		else if (string_equals_ignore_case(linea, KILL_ESI))
		{
			//Mata al ESI y libera sus recursos
			//todo funcion asesina
			linea = readline("CLAVE:");
			break;
		}
		else if (string_equals_ignore_case(linea, STATUS_ESI))
		{
			//todo funcion explicacion pendiente
			linea = readline("CLAVE:");
			//Explicacion pendiente
			break;
		}
		else if (string_equals_ignore_case(linea, COMPROBAR_DEADLOCK))
		{
			//devuelve las claves que esten generando deadlock (si es que lo hay)
			// aca va a haber una escucha del socket a la espera de la respuesta del plani
			break;
		}
		else if (string_equals_ignore_case(linea, "salir"))
		{
			break;
		}
		else
		{
			printf("escribi bien, gil");
			break;
		}

	}



}
