OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
INCLUDES := $(wildcard *.h)

all: fs_client fs_server

fs_client: fs_client.o fs_dummy.o

fs_server: fs_server.o simplified_rpc_lib
	gcc fs_server.o -L./simplified_rpc -lstubs -o Server 

simplified_rpc_lib:
	cd simplified_rpc && $(MAKE) all

$(OBJECTS): %.o: %.c $(INCLUDES)
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client
