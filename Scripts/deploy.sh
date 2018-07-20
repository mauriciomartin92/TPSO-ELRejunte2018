#!/bin/bash

#DEPLOY BIBLIOTECAS

clear

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/Scripts

chmod +x deployCommons.sh

#primero se deployan las coomons

echo "-------------------------------------------- INSTALANDO COMMONS ----------------------------------------------"

./deployCommons.sh

echo "-------------------------------------------- INSTALADAS ----------------------------------------------"

echo "------------------------------------------- INSTALANDO PARSI ------------------------------------"

./deployParsi.sh

echo "------------------------------------------- INSTALADO -------------------------------------------------"

# despues compilamos biblioteca propia

echo "-------------------------------------------- COMPILANDO BIBLIOTECA ----------------------------------------------"

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/biblioteca-El-Rejunte/Debug/

make clean

make all

echo "-------------------------------------------- COMPILADA ----------------------------------------------"

#se agregan las bibliotecas propias a las compartidas

echo "-------------------------------------------- INCLUYENDO BIBLIOTECA ----------------------------------------------"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2018-1c-El-Rejunte/biblioteca-El-Rejunte/Debug

echo "-------------------------------------------- EN PATH ----------------------------------------------" 

#compilamos el coordinador

echo "-------------------------------------------- COMPILANDO COORDINADOR ----------------------------------------------"

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/coordinador/Debug

make clean

make all

echo "-------------------------------------------- COMPILADO ----------------------------------------------"

# compilamos el planificador

echo "-------------------------------------------- COMPILANDO PLANIFICADOR ----------------------------------------------"

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/planificador/Debug

make clean

make all

echo "-------------------------------------------- COMPILADO ----------------------------------------------"

# compilamos la instancia

echo "-------------------------------------------- COMPILANDO INSTANCIA----------------------------------------------"

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/instancia/Debug

make clean

make all

echo "-------------------------------------------- COMPILADA----------------------------------------------"

# compilamos el ESI

echo "-------------------------------------------- COMPILANDO ESI----------------------------------------------"

cd /home/utnso/workspace/tp-2018-1c-El-Rejunte/esi/Debug

make clean

make all

echo " --------------------------------------------- COMPILADO-----------------------------------------------------"

echo $LD_LIBRARY_PATH
