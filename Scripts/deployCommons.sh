#! /bin/bash

#

# DEPLOY COMMONS 

#

cd ~

# Clonar repo Commons

git clone https://github.com/sisoputnfrba/so-commons-library.git

# Acceder a la carpeta

cd  so-commons-library/


# Compilar

make

# Instalar

echo "Ingrese password para instalar las commons..."

sudo make install

# chequeo que esten instaladas

ls -l /usr/include/commons