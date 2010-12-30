
all:
	(cd commands; make)
	(cd measure_bandw; make)

clean:
	(cd commands; make clean)
	(cd measure_bandw; make clean)

install:
	(cd commands; make install)
	(cd measure_bandw; make install)
