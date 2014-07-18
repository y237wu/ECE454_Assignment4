OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
INCLUDES := ece454_fs.h simplified_rpc/ece454rpc_types.h

all: fs_client fs_server client_api

fs_client: fs_client.o client_api
	gcc fs_client.o -L. -lclient_api -o fs_client

fs_server: fs_server.o simplified_rpc_lib
	gcc fs_server.o -L./simplified_rpc -lstubs -o Server 

client_api: simplified_rpc_lib fs_client_api.o
	ar r libclient_api.a fs_client_api.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o

simplified_rpc_lib:
	cd simplified_rpc && $(MAKE) all

$(OBJECTS): %.o: %.c $(INCLUDES)
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client Server
