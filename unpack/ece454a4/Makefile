server: fakeserver.c
	gcc fakeserver.c -o server

client-api.a: fakeclient-api.c
	gcc -c fakeclient-api.c
	ar r libclient-api.a fakeclient-api.o 

clean:
	rm -rf *.a *.o server a.out core
