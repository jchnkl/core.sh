PREFIX?=/usr

all: evagg

evagg: evagg.c
	gcc -g -std=gnu99 -Wall -O2 -lutil evagg.c -o evagg

install: evagg
	install -d ${PREFIX}/lib
	install -m 0755 evagg ${PREFIX}/lib
	install -d ${PREFIX}/bin
	install -m 0755 core.sh ${PREFIX}/bin
	sed -i "s|EVAGG=|EVAGG=${PREFIX}/lib/evagg|" ${PREFIX}/bin/core.sh
