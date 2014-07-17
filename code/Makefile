OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

all: fs_client fs_server

fs_client: fs_client.o fs_dummy.o

fs_server: fs_server.o
	gcc fs_server.o -o fsServer 

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client
