#include "planificador.h"

void recursoDestroy(t_recurso * recurso);
void lanzarConsola();



//char* rutaLog = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/Debug/logs";
char * KEY_PUERTO_CLIENTE = "PUERTO_CLIENTE";
char * KEY_ALGORITMO_PLANIFICACION = "ALGORITMO_PLANIFICACION";
char * KEY_ESTIMACION_INICIAL = "ESTIMACION_INICIAL";
char * KEY_IP_COORDINADOR = "IP_COORDINADOR";
char * KEY_PUERTO_COORDINADOR = "PUERTO_COORDINADOR";
char * KEY_IP = "IP";
char * KEY_PUERTO = "PUERTO";
char * KEY_CLAVES_BLOQUEADAS = "CLAVES_BLOQUEADAS";
char * KEY_CONSTANTE_ESTIMACION = "CONSTANTE_ESTIMACION";
char * RUTA_CONFIGURACION = "/home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/config_planificador.cfg";
char * SJF = "SJF";
char * HRRN = "HRRN";
char * SJFConDesalojo = "SJFConDesalojo";
char * HRRNConDesalojo = "HRRNConDesalojo";
char * ipPropia = "127.0.0.2";
char * puertoPropio = "8080";
int CONTINUAR = 1;
int FINALIZAR = 2;
int socketDeEscucha = 1; //conectarComoServidor(logPlanificador, ip, puerto,1);
uint32_t idESI = 0;
uint32_t GET = 0;
bool pausearPlanificacion = false;
bool matarESI=false;
int claveMatar = -1;

// CONSOLA


char * PAUSEAR_PLANIFICACION= "pausear_planificacion";
char* REANUDAR_PLANIFICACION = "reanudar_planificacion";
char* BLOQUEAR_ESI = "bloquear_esi";
char* DESBLOQUEAR_ESI = "desbloquear_esi";
char* LISTAR_POR_RECURSO = "listar_por_recurso";
char* KILL_ESI = "kill_esi";
char* STATUS_ESI = "status_esi";
char* COMPROBAR_DEADLOCK = "comprobar_deadlock";
pthread_mutex_t mutexColaListos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAsesino = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexComunicacion = PTHREAD_MUTEX_INITIALIZER;

ESI* estimarProximaRafaga(ESI * proceso ){


	log_info (logPlanificador, "EL ESI DE CLAVE %d TIENE RAFAGA ANTERIOR DE %d", proceso->id, proceso->rafagaAnterior);
	if(proceso -> rafagaAnterior == 0){

		proceso -> estimacionSiguiente = 0;

	} else{

		proceso->estimacionSiguiente = ((alfa*proceso->estimacionAnterior)+(1-alfa)*proceso->rafagaAnterior);

	}
	log_info(logPlanificador,"un tiempo estimado");
	return proceso;

}

t_deadlockeados * deadlockCreate(){

	t_deadlockeados * l = malloc(sizeof(t_deadlockeados));
	l->ESIasociados = list_create();
	return l;

}

bool compararClaves (ESI * esi){

		log_info(logPlanificador, "entra a la comparacion de claves");

		if(esi->id ==claveActual){

			return true;

		} else{

			return false;

		}

}


void comprobarDeadlock(){

	int contador = 0;
	t_list * dl = list_create();

	log_info (logPlanificador, "chequeando existencia de deadlock");

	while ( contador < list_size(listaRecursos) ){ // por cada recurso

		t_recurso * recursoAnalizar = list_get(listaRecursos, contador);

		log_info(logPlanificador,"analizando desde clave %s", recursoAnalizar->clave);

		int contador2 = 0;

		t_queue * colaRecurso = recursoAnalizar->ESIEncolados;

		while (contador2 < queue_size(recursoAnalizar->ESIEncolados)){ // tomo su primer ESI bloqueado

			ESI * Aux = queue_pop(colaRecurso);

			int contador3 = 0;

			while(list_size(Aux -> recursosAsignado) > contador3){ // y chequeo si para cada recurso asignado del mismo, hay un ESI en la cola de bloqueados del mismo que tenga asignada la clave que lo está bloqueando

				char * recursoAsignado = list_get(Aux->recursosAsignado,contador3);

				chequearDependenciaDeClave(recursoAnalizar->clave, recursoAsignado, Aux->id, dl );

				if(list_size(dl) > 0){ // en este caso se encontro el DL

					int cont = 0;

					printf("DEADLOCK formado por los siguientes ESI: \n");

					while(list_size(dl)> cont){

						printf("ESI %d \n", (int) list_get(dl,cont));

						cont ++;

						list_clean(dl); // limpio para encontrar otros DL
					}

				}

				}
			}

			contador2++;

		}

		contador++;


}


void chequearDependenciaDeClave(char * recursoOriginal, char * recursoESI, int idESI, t_list * listaDL){ //  hay que hacer una busqueda circular entre claves

	t_recurso * recurso = traerRecurso(recursoESI); // busco el recurso que iguale la clave del ESI

	if(recurso == NULL){ // Si no lo encuentra, no mando nada, no deberia pasar, logueo para estar al tanto

		log_info(logPlanificador,"CASO RARO: no se encontro el recurso que tiene asignado un ESI");

	} else { // al encontrar el recurso, tengo que chequear por cada ESI bloqueado de la clave, si su asignado coincide con la clave del recursoOriginal

		int i = 0;

		t_queue * colaESIS = recurso->ESIEncolados; // creo cola para no modificar la cola de esis original

		bool DLEncontrado = false;

		while ( queue_size(colaESIS) > i && !DLEncontrado){ // por cada ESI encolado

			ESI * esi = queue_pop(recurso->ESIEncolados); // tomo de a uno

			int t = 0;

			while (list_size(esi->recursosAsignado) > t && !DLEncontrado){

				char * recurso = list_get (esi->recursosAsignado,t); // saco un recurso asignado

				if(string_equals_ignore_case(recurso,recursoOriginal)){ // es igual al recurso original?

					list_add(listaDL,&idESI);
					list_add(listaDL, &esi->id);
					DLEncontrado = true;

				} else { // si no son iguales, va a buscar coincidencia en los recursos que ese esi tiene asignados

					chequearDependenciaDeClave(recursoOriginal, recurso, esi->id, listaDL); //recursivo, el recu original y el nuevo

					if(list_size(listaDL)> 0){ // si metio algo en lista, quiere decir que encontro espera circular, que dada la unicidad de ESI por recurso, va a generar un ciclo de un solo esi por recurso
						list_add(listaDL, &idESI); // agrego el primer esi, que por recursividad, seria el tercero del dl
						DLEncontrado = true; // por unicidad de recursos, no puede haber mas de un ciclo asociado a un recurso.
					}

				}

				t++;


			}

			i++;

		}

	}

}


t_recurso * traerRecurso (char * clave){


	t_recurso * recurso;
	bool encontrado = false;
	int i = 0;

	while(list_size(listaRecursos) > i && !encontrado){

		recurso = list_get(listaRecursos, i);

		if(recurso->clave == clave){
			encontrado = true;
		}
		i++;
	}

	if(encontrado == false){
		recurso = NULL;
	}

	return recurso;
}
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
		printf("Salir \n");
		printf("Ingrese el nombre de la opcion, incluyendo el guion bajo \n");
		linea = readline(">");

		if (string_equals_ignore_case(linea, PAUSEAR_PLANIFICACION))  //Hermosa cadena de if que se viene
		{
			pausearPlanificacion = true;
		}
		else if (string_equals_ignore_case(linea,REANUDAR_PLANIFICACION))
		{
			pausearPlanificacion= false;
		}
		else if (string_equals_ignore_case(linea, BLOQUEAR_ESI))
		{
			linea = readline("CLAVE ESI:");
			int n = strlen(linea);

			int i = 0;
			int aux = 1;

			int clave =0;

			while (n < i){
				aux= aux *10;
				i++;
			}

			i= 0;

			while (i < n){

				aux =aux/10;
				clave = clave + ((linea[i] - '0') * (aux));
				i++;

			}
			linea = readline("CLAVE RECURSO:");

			if( claveActual==clave){

				claveParaBloquearESI = clave;
				string_append(&claveParaBloquearRecurso, linea);

			} else {

				ESI * nuevoESI = buscarESI(clave);

				if(nuevoESI == NULL){
					printf("se introdujo una clave erronea : %d, error, autodestruir ", clave);
					exit(-1);
				} else {

					bloquearESI(linea, nuevoESI);
				}
			}

		}
		else if (string_equals_ignore_case(linea,DESBLOQUEAR_ESI))
		{
			linea = readline("CLAVE RECURSO:");
			desbloquearRecurso(linea);


		}
		else if (string_equals_ignore_case(linea, LISTAR_POR_RECURSO)){

			linea = readline("RECURSO:");
			listarBloqueados(linea);
		}
		else if (string_equals_ignore_case(linea, KILL_ESI))
		{

			linea = readline("CLAVE ESI:");
			int n = strlen(linea);

			int i = 0;
			int aux = 1;

			int clave =0;

			while (n < i){
				aux= aux *10;
				i++;
			}

			i= 0;

			while (i < n){

				aux =aux/10;
				clave = clave + ((linea[i] - '0') * (aux));
				i++;

			}

			if(clave == claveActual){

				log_info(logPlanificador, "la clave del esi ejecutandose es igual a la que se quiere matar");
				pthread_mutex_lock(&mutexAsesino);

				matarESI = true;

				pthread_mutex_unlock(&mutexAsesino);

				printf("esperando a que el ESI pueda ser matado");

				while(1){
				if(matarESI == false){
				}

				}

				log_info(logPlanificador, "se termino con el esi");
				printf("esi muerto y liberados sus recursos");

			} else {
				claveMatar = clave;
				seekAndDestroyESI(clave);
			}

		}
		else if (string_equals_ignore_case(linea, STATUS_ESI))
		{

			linea = readline("CLAVE:");
			statusClave(linea);

		}
		else if (string_equals_ignore_case(linea, COMPROBAR_DEADLOCK))
		{
			comprobarDeadlock(); // hace printf en la funcion
			break;
		}
		else if (string_equals_ignore_case(linea, "salir"))
		{
			break;
		}
		else
		{
			printf("Comando no reconocido");
		}

	}
}


ESI * crearESI(uint32_t clave){

	ESI * nuevoESI = malloc(sizeof(ESI));
	nuevoESI->id = clave;
	nuevoESI->estimacionAnterior= estimacionInicial;
	nuevoESI-> bloqueadoPorUsuario = false;
	nuevoESI-> rafagaAnterior = 0;
	nuevoESI-> estimacionSiguiente = 0;
	nuevoESI->rafagasRealizadas =0;
	nuevoESI-> tiempoEspera = 0;
	nuevoESI->recursosAsignado= list_create();
	nuevoESI->recursoPedido = string_new();
	nuevoESI->proximaOperacion = -1;
	nuevoESI->recienLlegado = true;
	nuevoESI->recienDesbloqueadoPorRecurso = false;

	return nuevoESI;

}

void escucharNuevosESIS(){

	while(1){

		uint32_t socketESINuevo = escucharCliente(logPlanificador,socketDeEscucha);
		send(socketESINuevo,&socketESINuevo,sizeof(uint32_t),0);
		ESI * nuevoESI = crearESI(socketESINuevo);
		list_add(listaListos,nuevoESI);

		pthread_mutex_lock(&mutexColaListos); //pongo mutex para que no se actualice la cola mientras agrego un ESI.
		if(string_equals_ignore_case(algoritmoDePlanificacion,SJF) || string_equals_ignore_case(algoritmoDePlanificacion,SJFConDesalojo)){
			armarColaListos();
		} else if(string_equals_ignore_case(algoritmoDePlanificacion, HRRN) || string_equals_ignore_case(algoritmoDePlanificacion,HRRNConDesalojo)){
			armarCola();
		}
		pthread_mutex_unlock(&mutexColaListos);

	}
}


t_recurso * crearRecurso (char * id){

	t_recurso * nuevo = malloc(sizeof(t_recurso));
	nuevo->estado = 0;
	nuevo->clave = string_new();
	string_append(&(nuevo->clave), id);
	nuevo->operacion = 0;
	nuevo->valor = string_new();
	nuevo->ESIEncolados = queue_create();
	return nuevo;
}

void ESI_destroy(ESI * estructura)
{
		list_destroy_and_destroy_elements(estructura->recursosAsignado, (void * ) free);
		free(estructura->recursoPedido);
		free(estructura);
}


void recursoDestroy(t_recurso * recurso)
{
	free(recurso->clave);
	free(recurso->valor);
	queue_destroy_and_destroy_elements(recurso->ESIEncolados, (void *) ESI_destroy);
	free(recurso);

}



void DEADLOCK_destroy(t_deadlockeados * ESI){

	list_destroy(ESI->ESIasociados);
	free(ESI);

}

bool validarPedido (char * recurso, ESI * ESIValidar){

	int i = 0;
	bool encontrado = false;
	bool retorno = false;

	log_info(logPlanificador,"verificando si el proceso tiene permiso de ejecutar");

	while(list_size(listaRecursos) > i){

			t_recurso * nuevo = list_get(listaRecursos, i);

			if(string_equals_ignore_case(recurso,nuevo->clave)){

				encontrado = true;

				log_info(logPlanificador,"el recurso pedido existe");

				if(nuevo->estado == 1 && ESIValidar->proximaOperacion == 1){ // caso recurso tomado para get

					log_info(logPlanificador,"Se quiere hacer un GET de un recurso tomado. Se le niega permiso al ESI");
					nuevo = NULL;
					retorno = false;

				} else if (nuevo-> estado == 1 && ESIValidar-> proximaOperacion > 1){ //caso recurso tomado para set y store

					log_info(logPlanificador,"se quiere hacer un SET o STORE. Se chequea si la clave esta asignada");
					bool RecursoEncontrado = false;
					int t = 0;
					while(list_size(ESIValidar->recursosAsignado) > t){

						char * recursoAuxiliar = list_get (ESIValidar->recursosAsignado, t);

						if( string_equals_ignore_case(recursoAuxiliar,nuevo->clave)){ // caso ESI tiene asignado recurso

							log_info(logPlanificador,"clave asignada. Tiene permiso");
							RecursoEncontrado = true;
							nuevo->operacion = ESIValidar->proximaOperacion; //mete si hace set o get
							retorno = true;
						}

						recursoAuxiliar= NULL;

						t++;

					} if(RecursoEncontrado == false){ // Caso ESI no tiene asignado recurso

						log_info(logPlanificador," No se puede hacer un SET o STORE de un recurso no asignado!");
						nuevo = NULL;
						retorno = false;

					}


				} else if(nuevo->estado==0 && ESIValidar->proximaOperacion > 1){ // caso el recurso no esta tomado y quiere hacer SET o STORE

					log_info(logPlanificador,"SET o STORE de recurso no tomado, no se permite");
					retorno = false;
				} else {

					log_info(logPlanificador,"GET de un recurso no tomado. Se permite");
					retorno = true;

				}

			}

			i++;

		}

		if ( encontrado == false){ // caso recurso nuevo

			log_info(logPlanificador,"Se crea recurso nuevo porque no fue encontrada la clave pedida. Tiene permiso");
			t_recurso * nuevoRecurso = crearRecurso(recurso);
			nuevoRecurso -> operacion =  0;
			list_add(listaRecursos,nuevoRecurso);
			retorno = true;

		}

		return retorno;
}


/**
 *
 * Bloquea un recurso cuando se pone en uso.
 * De no existir la clave que llega, crea el recurso y lo mete en lista.
 *
 */
void bloquearRecurso (char* claveRecurso) {


	int i = 0;
	bool encontrado = false;
	while(list_size(listaRecursos) > i && !encontrado)
	{
		t_recurso * nuevoRecurso = list_get(listaRecursos,i);
		if(string_equals_ignore_case(nuevoRecurso->clave,claveRecurso))
		{
			if(nuevoRecurso->estado == 0){

				nuevoRecurso->estado = 1;
				log_info(logPlanificador, "Recurso de clave %s bloqueado", nuevoRecurso->clave);
				encontrado = true;

			} else {
				log_info(logPlanificador, " se intento bloquear un recurso ya bloqueado. Se ignora");
			}

		}
		i++;
	}

}

void desbloquearRecurso (char* claveRecurso) {

	int i = 0;
	bool encontrado = false;
	while(list_size(listaRecursos) > i && !encontrado)
	{
		t_recurso * nuevoRecurso = list_get(listaRecursos,i);
		if(string_equals_ignore_case(nuevoRecurso->clave,claveRecurso))
		{
			if(nuevoRecurso->estado == 1){ // libera el recurso y saca de la cola a SOLO UN ESI que lo estaba esperando

				nuevoRecurso->estado = 0;
				encontrado = true;
				ESI * nuevo = queue_pop(nuevoRecurso->ESIEncolados);
				nuevo->recienDesbloqueadoPorRecurso = true;
				log_info(logPlanificador, "Recurso de clave %s desbloqueado", nuevoRecurso->clave);
				list_add(listaListos,nuevo);

			} else {
				log_info(logPlanificador, " se intento desbloquear un recurso no bloqueado. Se ignora");
			}

		}
		i++;
	}

}

void bloquearESI(char * claveRecurso, ESI * esi){

	int i = 0;
	bool encontrado = false;
	while(i< list_size(listaRecursos) && !encontrado){

		t_recurso * recursoAuxiliar = list_get(listaRecursos, i);
		if(string_equals_ignore_case(recursoAuxiliar->clave,claveRecurso)){
			queue_push(recursoAuxiliar->ESIEncolados,esi);
			encontrado = true;

			i++;
		}

	}

}

bool recursoEnLista(ESI * esi){

	log_info(logPlanificador,"chequeo si el recurso está en su lista de asignados");
	bool retorno = false;
	int i = 0;
	while( i< list_size(esi->recursosAsignado) && !retorno){
		char * r = list_get(esi->recursosAsignado, i);

		if(string_equals_ignore_case(r, esi->recursoPedido)){
			retorno = true;
			log_info(logPlanificador,"Está");
		}
		i++;
	}

	return retorno;
}

void liberarRecursos(ESI * esi){

	int i=0;

	log_info(logPlanificador,"liberando recursos del ESI");

	while(i < list_size(esi->recursosAsignado)){

		char * recurso = list_get(esi->recursosAsignado, i);
		desbloquearRecurso( recurso );
		i++;
	}

	log_info(logPlanificador,"Terminado");

	list_clean(esi->recursosAsignado);

}

void limpiarRecienLlegados(){

	log_info(logPlanificador, "actualizando cola listos");
	t_queue * colaAuxiliar = queue_create();

	while(queue_is_empty(colaListos)){

		ESI*nuevo = queue_pop(colaListos);

		nuevo -> recienLlegado = false;

		queue_push(colaAuxiliar, nuevo);

	}

	log_info(logPlanificador,"Cola de listos al dia");

	colaListos = colaAuxiliar;
	queue_destroy(colaAuxiliar);
}

ESI * buscarESI(int clave){

	int i=0;
	bool e = false;
	ESI * encontrado;
	log_info(logPlanificador,"se busca un ESI de id %d", clave);

	while(i <= list_size(listaListos) && !e){

		ESI * aux = list_get(listaListos,i);
		if(aux-> id == clave){

			log_info(logPlanificador,"Encontrado");
			encontrado = list_remove(listaListos, i);
			e=true;

		}
	}

	if (e == false){
		log_info(logPlanificador,"No se encontró");
		encontrado = NULL;
	}

	return encontrado;
}


void listarBloqueados(char * clave){

	int i = 0;
	t_recurso * encontrado;
	bool e = false;

	while (list_size(listaRecursos) > i && !e){

		encontrado = list_get (listaRecursos, i);
		if(string_equals_ignore_case(encontrado->clave,clave)){
			e = true;
		}
		i++;

	}

	if (e == false){

		printf("no se encontro el recurso de clave %s  \n", clave);

	} else {

		int x = queue_size(encontrado->ESIEncolados);

		if(x == 0){

			printf("la clave no tiene ESI en cola  \n");

		}else{

			int i=0;
			t_queue * colaAuxiliar = queue_create();
			colaAuxiliar = encontrado->ESIEncolados;
			printf("esis en cola:");
			while(x > i){

				ESI * esi = queue_pop(colaAuxiliar);
				printf("ESI: %d  \n", esi->id);
				i++;

			}
			queue_destroy(colaAuxiliar);
		}
	}


}

void seekAndDestroyESI(int clave){

	log_info(logPlanificador,"Buscando a ESI de clave %d", clave);

	ESI * aDestruir = buscarESI(clave);

	if(aDestruir == NULL){

		bool estaEnBloqueados = buscarEnBloqueados(clave);

		if(estaEnBloqueados == false){

			printf("No existe tal ESI \n");

		} else printf("ESI muerto \n");

	} else{
		log_info(logPlanificador,"ESI encontrado en cola de Listos y terminado.");
		liberarRecursos(aDestruir);
		list_remove_and_destroy_by_condition(listaListos, (void *)encontrarVictima, (void *) ESI_destroy);
		printf("ESI muerto");
		claveMatar = -1;

	}



}


bool encontrarVictima (ESI * esi){

		log_info(logPlanificador, "entra a la comparacion de claves");

		if(esi->id ==claveMatar){

			return true;

		} else{

			return false;

		}

}

void statusClave(char * clave){

	int i = 0;
	bool encontrado = false;

	log_info(logPlanificador, "Obteniendo status de una clave");

	while (i < list_size(listaRecursos) && !encontrado){

		t_recurso * recurso= list_get (listaRecursos, i);

		if (string_equals_ignore_case(recurso->clave, clave)){

			log_info(logPlanificador, "clave encontrada");
			if(string_is_empty(recurso->valor)){
				log_info(logPlanificador, "sin valor");
				printf("valor : NO TIENE \n");

		} else {
			log_info(logPlanificador, "valor : %s \n", recurso->valor);
			printf ("valor : %s \n", recurso->valor);
		}

		pthread_mutex_lock(&mutexComunicacion);

		log_info(logPlanificador, "Pidiendo instancia de clave");
		uint32_t traemeLaClaveCoordi = 4;
		send(claveActual, &traemeLaClaveCoordi, sizeof(uint32_t),0);
		send(claveActual, clave, sizeof(clave),0);

		uint32_t tam;
		char * instancia;

		int resp = recv(socketCoordinador,&tam, sizeof(uint32_t), 0);
		instancia = malloc(sizeof(char)*tam);
		int resp2= recv(socketCoordinador, instancia, sizeof(char)*tam,0);

		pthread_mutex_unlock(&mutexComunicacion);

		if (resp < 0 || resp2 <0){

			log_info(logPlanificador, " fallo conexion");
			exit (-1);

		} else {
			log_info(logPlanificador, "instancia de clave %s",instancia);
			printf(" instancia : %s \n", instancia);
		}

		free(instancia);
		listarBloqueados(clave);
		encontrado = true;

		} else i++;

	}
	if(!encontrado){
		log_info(logPlanificador, "clave no hallada");
		printf("la clave no fue encontrada \n");
	}

}


extern void cargarValor(char* clave, char* valor){

	int i = 0;
	bool encontrado = false;
	log_info(logPlanificador, "cargando valor a clave");
	while(i< list_size(listaRecursos) && !encontrado){

		t_recurso * auxiliar = list_get(listaRecursos, i);

		if(string_equals_ignore_case(auxiliar->clave, clave)){

			log_info(logPlanificador, "clave encontrada");
			string_append(&(auxiliar->valor),valor);
			log_info(logPlanificador, "valor nuevo: %s",auxiliar->valor);
			encontrado = true;
		}

		i++;


	}

	if(!encontrado){

		log_info(logPlanificador,"no encontro la clave");
	}
}

bool buscarEnBloqueados (int clave){

	int i = 0;
	bool encontrado = false;

	log_info(logPlanificador, "arranca busqueda en bloqueados");

	while (list_size( listaRecursos) > i && !encontrado){

		t_recurso * recu = list_get(listaRecursos, i);

		int r = 0;

		t_queue * cola = recu->ESIEncolados; // la cola esta apuntando a los bloqueados

		while(queue_size(recu->ESIEncolados)> r && !encontrado){

			ESI * aux = queue_pop(cola); // avanza entre bloqueados

			if(aux->id == clave){ // si encuentra, libera recursos y destruye al ESI

				log_info(logPlanificador, "se encontro el esi");
				encontrado = true;
				liberarRecursos(aux);
				ESI_destroy(aux);
				claveMatar=-1;
			}

			r++;

		} // pero al encontrarlo, la cola original queda con un hueco que generara fallas mas adelante

		if(encontrado){ // entonces si fue encontrado

			int t = 0;
			log_info(logPlanificador, "rearmando cola de bloqueados de la clave");

			t_queue * colaNueva = queue_create(); //creo una cola nueva

			while(queue_size(recu->ESIEncolados)>t ){

				ESI * esiComprobar = queue_pop(recu->ESIEncolados); // voy sacando de a uno de la original

				if(esiComprobar != NULL){ //si es diferente de NULL

					queue_push(colaNueva,esiComprobar); // lo meto en la cola nueva
				} // si no, no hago nada

			}

			recu->ESIEncolados = colaNueva; // ahora la original apunta a la nueva, que no tiene el hueco.
			log_info(logPlanificador, "rearmada");
			queue_destroy(colaNueva); //destruyo la cola nueva pero no los elementos
		} // cuanto mas facil hubiese sido con una lista..-
		i++;

	}


	return encontrado;
}
