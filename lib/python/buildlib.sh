#!/bin/sh

export PATH=$PATH:/usr/local/OPEN_R_SDK/bin

if [ ! -e /usr/local/OPEN_R_SDK/mipsel-linux/lib/crt1.o ]; then
	echo "You need to install the OPEN_R_SDK extras"
fi

mkdir build-host
rm -rf build-openr
mkdir build-openr
mkdir mipsel-python

DESTDIR=$(pwd)/mipsel-python

cp src/configure src/configure.mips
patch -p0 < conf.patch || exit

cd build-host

../src/configure
make Parser/pgen

cd ../build-openr
env CC=mipsel-linux-gcc CPP=mipsel-linux-cpp CXX=mipsel-linux-g++ STRIP=mipsel-linux-strip RANLIB=mipsel-linux-ranlib AR=mipsel-linux-ar CPPFLAGS=-I/usr/local/OPEN_R_SDK/OPEN_R/include MACHDEP=unknown ../src/configure.mips --target=mipsel-linux --prefix=$DESTDIR --disable-shared

patch -p0 < ../pyconfig.patch || exit

patch -p0 < ../Setup.patch || exit

patch -p0 < ../Makefile.patch || exit

make -W Makefile.pre Makefile
make libpython2.3.a
make inclinstall
cp libpython2.3.a ../mipsel-python/

