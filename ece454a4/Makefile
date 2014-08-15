OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
INCLUDES := ece454_fs.h simplified_rpc/ece454rpc_types.h fsOtherIncludes.h

all: client-api.a server

server: fs_server.o simplified_rpc_lib
	gcc fs_server.o -L./simplified_rpc -lstubs -o server 

client-api.a: simplified_rpc_lib fs_client_api.o
	ar r libclient-api.a fs_client_api.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o

simplified_rpc_lib:
	cd simplified_rpc && $(MAKE) all

$(OBJECTS): %.o: %.c $(INCLUDES)
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client server
