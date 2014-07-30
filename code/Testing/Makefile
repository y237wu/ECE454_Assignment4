OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
INCLUDES := ece454_fs.h fsOtherIncludes.h

all: fs_client fs_client3 fs_client4 fs_client5 fs_client6 fs_client7

fs_client: fs_client.o libclient_api.a
	gcc fs_client.o -L. -lclient_api -o fs_client

fs_client3: fs_client2Servers1Client.o libclient_api.a
	gcc fs_client2Servers1Client.o -L. -lclient_api -o fs_client3

fs_client4: fs_clientTest1Basic.o libclient_api.a
	gcc fs_clientTest1Basic.o -L. -lclient_api -o fs_client4

fs_client5: fs_clientTest2WriteOpenReadCloseDir.o libclient_api.a
	gcc fs_clientTest2WriteOpenReadCloseDir.o -L. -lclient_api -o fs_client5

fs_client6: fs_clientTest3Concurrency1.o libclient_api.a
	gcc fs_clientTest3Concurrency1.o -L. -lclient_api -o fs_client6

fs_client7: fs_clientTest3Concurrency2.o libclient_api.a
	gcc fs_clientTest3Concurrency2.o -L. -lclient_api -o fs_client7

$(OBJECTS): %.o: %.c $(INCLUDES)
	gcc -c $< -o $@
