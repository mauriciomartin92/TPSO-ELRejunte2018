# tp-2018-1c-El-Rejunte

Para compilar un proceso, por ejemplo coordinador, hay que hacer esto:

> cd workspace/tp-2018-1c-El-Rejunte/coordinador/Debug/
> make clean
> make
> export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2018-1c-El-Rejunte/biblioteca-El-Rejunte/Debug/
> ./coordinador

Observación: para ver a donde quiere linkear la biblioteca utilizar
> ldd coordinador

REEDIT: CONFIGURÉ EL .bashrc PARA QUE NO HAGA FALTA EL "EXPORT", LO OTRO SÍ. ENTONCES QUEDA:

> cd workspace/tp-2018-1c-El-Rejunte/coordinador/Debug/
> make clean
> make
> ./coordinador
