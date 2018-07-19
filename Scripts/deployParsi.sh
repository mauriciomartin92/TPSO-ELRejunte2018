#! /bin/bash

#

# DEPLOY PARSER 

#

cd ~

# Clonar repo Commons

git clone https://github.com/sisoputnfrba/parsi.git

# Acceder a la carpeta

cd  parsi/

# Compilar

make

# Instalar

echo "Ingrese password para instalar las commons..."

sudo make install

# chequeo que esten instaladas

ls -l /usr/include/parsi