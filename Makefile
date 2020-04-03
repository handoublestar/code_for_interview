all:test	
test:main.o
	gcc -o test main.o -I. -L. -lsocketserver -Wl,-rpath=.
main.o:main.c
	gcc -c -fPIC main.c
lib:
	gcc -c -fPIC socketserver.c
	gcc -shared -o libsocketserver.so socketserver.o
clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf test
