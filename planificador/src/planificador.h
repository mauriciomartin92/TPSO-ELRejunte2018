#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "../../biblioteca-El-Rejunte/src/misSockets.h"
#include <readline/readline.h>
#include <pthread.h>





//constantes de planificación

extern char * SJF;
extern char * HRRN;
extern char * SJFConDesalojo;
extern char * HRRNConDesalojo;
extern char * RUTA_CONFIGURACION ;


//keys para cfg
extern char * KEY_PUERTO_CLIENTE;
extern char * KEY_ALGORITMO_PLANIFICACION ;
extern char * KEY_ESTIMACION_INICIAL ;
extern char * KEY_IP_COORDINADOR ;
extern char * KEY_PUERTO_COORDINADOR;
extern char * KEY_IP;
extern char * KEY_PUERTO;
extern char * KEY_CLAVES_BLOQUEADAS ;
extern char * KEY_CONSTANTE_ESTIMACION ;
extern char * PAUSEAR_PLANIFICACION;
extern char* REANUDAR_PLANIFICACION ;
extern char* BLOQUEAR_ESI ;
extern char* DESBLOQUEAR_ESI ;
extern char* LISTAR_POR_RECURSO ;
extern char* KILL_ESI ;
extern char* STATUS_ESI ;
extern char* COMPROBAR_DEADLOCK;


// SOCKETS


extern int socketDeEscucha;
extern char * ipPropia;
extern char * puertoPropio;
extern int backlog;
extern int CONTINUAR;
extern int FINALIZAR;
extern uint32_t idESI;

// GLOBALES


char * algoritmoDePlanificacion;
int rafaga;
t_config * archivoConfiguracion;
t_log * logPlanificador;
t_queue * colaListos;
t_queue * colaBloqueados; // nota importante: uso una sola para los bloqueados. No le veo sentido tener varias
t_list * listaListos;
t_list * listaFinalizados;
t_list * listaRecursos;
int puertoEscucha;
int alfa;
int estimacionInicial;
char * ipCoordinador;
char * puertoCoordinador;
char ** clavesBloqueadas;
extern char * rutaLog;
bool recursoGenericoEnUso;
pthread_t  hiloEscuchaConsola;
pthread_t hiloEscuchaESI;
pthread_t * hiloPlanifica;
int claveActual;
t_list * deadlockeados;
int claveParaBloquearESI;
char * claveParaBloquearRecurso;
int socketCoordinador;
extern pthread_mutex_t mutexColaListos;
extern pthread_mutex_t mutexAsesino;
extern pthread_mutex_t mutexComunicacion;
extern bool pausearPlanificacion;
extern bool matarESI;
extern int claveMatar;

// ESTRUCTURAS DE PROCESOS

typedef struct{

	uint32_t id;
	int rafagasRealizadas;
	int rafagaAnterior;
	int estimacionAnterior;
	int estimacionSiguiente;
	int tiempoEspera;
	bool bloqueadoPorUsuario;
	bool recienDesbloqueadoPorRecurso;
	t_list * recursosAsignado;
	char * recursoPedido;
	int proximaOperacion;
	bool recienLlegado;
	float tiempoRespuesta;
}ESI;

typedef struct{ // en la lista de subrecursos: futbol, basquet..

	int estado;
	char * clave;
	int operacion;
	t_queue * ESIEncolados;
	char * valor;

} t_recurso;

typedef struct{

	ESI * bloqueado;
	char * claveRecurso;

} t_ESIBloqueado;


typedef struct{

	char * clave;
	t_list * ESIasociados;

} t_deadlockeados;



//FUNCIONES

void configurar ();
void liberarGlobales ();
void estimarRafagaSiguiente(int tiempoAnterior);
ESI * crearESI(uint32_t clave);
void ESI_destroy(ESI * estructura);
void liberarRecursos(ESI * esi);
ESI* estimarProximaRafaga(ESI* proceso );
bool compararClaves (ESI * esi);
void comprobarDeadlock();
void DEADLOCK_destroy(t_deadlockeados * ESI);
t_recurso * crearRecurso (char * id);
void crearSubrecurso (char* claveRecurso, char * claveSubrecurso);
extern void recursoDestroy(t_recurso * recurso);
extern void lanzarConsola();
extern void bloquearESI(char * claveRecurso, ESI * esi);
extern void escucharNuevosESIS();
extern void bloquearRecurso(char * claveRecurso);
extern void desbloquearRecurso(char * claveRecurso);
extern bool validarPedido (char * recurso, ESI * esi);
extern bool recursoEnLista(ESI * esi);
extern void limpiarRecienLlegados();
extern ESI * buscarESI(int clave);

extern void planificacionSJF(bool desalojo);
extern void estimarTiempos();
extern void armarColaListos();


extern void planificacionHRRN(bool desalojo);
extern void estimarYCalcularTiempos();
extern float calcularTiempoEspera (float espera, int estimacionSiguiente);
extern void armarCola ();
extern void sumarTiemposEspera ();
extern void aumentarEspera();

extern void listarBloqueados(char * clave);
extern bool encontrarVictima (ESI * esi);
extern void seekAndDestroyESI(int clave);
extern void statusClave(char * clave);
extern void cargarValor(char * clave, char * valor);
bool buscarEnBloqueados(int id);

#endif /* PLANIFICADOR_H_ */
