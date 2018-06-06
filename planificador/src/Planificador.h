#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>


//constantes de planificación

extern char * SJF;
extern char * HRRN;
extern char * RUTA_CONFIGURACION ;


//keys para cfg
extern char * KEY_PUERTO_CLIENTE;
extern char * KEY_ALGORITMO_PLANIFICACION ;
extern char * KEY_ESTIMACION_INICIAL ;
extern char * KEY_IP_COORDINADOR ;
extern char * KEY_PUERTO_COORDINADOR;
extern char * KEY_CLAVES_BLOQUEADAS ;
extern char * KEY_CONSTANTE_ESTIMACION ;


// GLOBALES


char * algoritmoDePlanificacion;
int rafaga;
t_config * archivoConfiguracion;
t_log * logPlanificador;
t_queue * colaListos;
t_list * listaListos;
t_list * listaBloqueados;
t_list * listaFinalizados;
t_list * listaRecursos;
int puertoEscucha;
int alfa;
int estimacionInicial;
char * ipCoordinador;
int puertoCoordinador;
char ** clavesBloqueadas;
extern char * rutaLog;
bool recursoGenericoEnUso;
pthread_t  hiloEscucha;
pthread_t * hiloPlanifica;
char * claveActual;
t_list * deadlockeados;

// ESTRUCTURAS DE PROCESOS

typedef struct{

	char * id;
	int rafagasRealizadas;
	int rafagaAnterior;
	int estimacionAnterior;
	int estimacionSiguiente;
	float tiempoEspera;
	bool bloqueadoPorRecurso;
	bool bloqueadoPorUsuario;
	char * recursoAsignado; // clave
	char * recursoPedido; // clave

}ESI;


typedef struct{ // esta en la lista de recursos: materia, deporte ..

	char * clave;
	t_list * subrecursos;

} t_recurso;

typedef struct{ // en la lista de subrecursos: futbol, basquet..

	char * clave;
	t_list * recursosFinales;

} t_subrecurso;

typedef struct{ // en la lista de recursosFinales: futbolLeoMessi, futbolLuisSuarez ..

	char * clave;
	char * valor;

} t_recursoFinal;


typedef struct{

	char * clave;
	t_list * ESIasociados;

} t_deadlockeados;



//FUNCIONES

void configurar ();
void liberarGlobales ();
void estimarRafagaSiguiente(int tiempoAnterior);
ESI * crearESI(char* clave);
void ESI_destroy(ESI * estructura);
void escucharPedidos();
void liberarRecursos(int recursoID);
void estimarProximaRafaga(ESI* proceso );
bool compararClaves (ESI * esi);
void comprobarDeadlock();
void DEADLOCK_destroy(t_deadlockeados * ESI);
t_recurso * crearRecurso (char * id);
void crearSubrecurso (char* claveRecurso, char * claveSubrecurso);
void recursoDestroy(t_recurso * recurso);
void subrecursoDestroy (t_subrecurso * subrecurso);
void recursoFinalDestroy(t_recursoFinal * recuFinal);



#endif /* PLANIFICADOR_H_ */
