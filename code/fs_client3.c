#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

void printBuf(char *buf, int size) {
    /* Should match the output from od -x */
    int i;
    for(i = 0; i < size; ) {
	if(i%16 == 0) {
	    printf("%08o ", i);
	}

	int j;
	for(j = 0; j < 16;) {
	    int k;
	    for(k = 0; k < 2; k++) {
		if(i+j+(1-k) < size) {
		    printf("%02x", (unsigned char)(buf[i+j+(1-k)]));
		}
	    }

	    printf(" ");
	    j += k;
	}

	printf("\n");
	i += j;
    }
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
	fprintf(stderr, "usage: %s <srv-ip/name> <srv-port>\n", argv[0]);
	exit(1);
    }

    char dirname[256] = "dir1";
    printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
    FSDIR *fd = fsOpenDir(dirname);
    if(fd == NULL) {
	perror("fsOpenDir"); exit(1);
    }

    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
	printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    if(errno != 0) {
	perror("fsReadDir");
    }

    char fname[256];
    memset(fname, 0, 256);
    strcpy(fname, dirname);
    strcat(fname, "/concurrentTest.txt");

    int ff = fsOpen(fname, 1);
    if(ff < 0) {
	perror("fsOpen(write)"); exit(1);
    }

    char buf[256] = "test\n";
    if(fsWrite(ff, buf, 256) < strlen(buf)) {
	fprintf(stderr, "fsWrite() wrote fewer than %d\n", strlen(buf));
    }

    printf("sleeping 10 seconds to block any access on %s\n", fname);
    sleep(10);

    if(fsClose(ff) < 0) {
	perror("fsClose"); exit(1);
    }

    printf("fsCloseDir(): %d\n", fsCloseDir(fd));

    sleep(5);

    printf("fsRead should be blocked\n");
    char readbuf[256];
    if((ff = fsOpen(fname, 0)) < 0) {
	perror("fsOpen(read)"); exit(1);
    }
    printf("fsOpen ff %d\n", ff);

    int readcount = -1;

    readcount = fsRead(ff, readbuf, 256);
    printf("fsRead() read %d bytes\n", readcount);
    printf("readbuf: %s\n", readbuf);

    printf("blocking remove for 10 seconds\n");
    sleep(10);

    if(fsClose(ff) < 0) {
	perror("fsClose"); exit(1);
    }

    if(fsUnmount(dirname) < 0) {
	perror("fsUnmount"); exit(1);
    }

    return 0;
}
