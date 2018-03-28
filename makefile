CC = gcc -std=gnu99  -fprofile-arcs -ftest-coverage

FSeed:main.o signal.o http.o  rio.o threadpool.o
	$(CC) *.o -o FSeed -lpthread

main.o:main.c
	$(CC) -c main.c

threadpool.o:threadpool.c
	$(CC) -c threadpool.c


signal.o:signal.c signal.h
	$(CC) -c signal.c

http.o:http.c http.h
	$(CC) -c http.c

rio.o:rio.c rio.h
	$(CC) -c rio.c

clean:
	rm *.o *.gcda *.gcno  FSeed 
