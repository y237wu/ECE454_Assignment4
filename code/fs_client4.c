#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

int main(int argc, char *argv[]) {
    if(argc < 3) {
	fprintf(stderr, "usage: %s <srv-ip/name> <srv-port>\n", argv[0]);
	exit(1);
    }

    char dirname[256] = "dir2";
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

    printf("opening file %s should be blocked\n", fname);
    int ff = fsOpen(fname, 1);
    if(ff < 0) {
	perror("fsOpen(write)"); exit(1);
    }

    printf("file opened this should block the read to %s\n", fname);

    char buf[256] = "test2\n";
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

    printf("should be blocked. going to remove file %s\n", fname);
    printf("fsRemove(%s): %d\n", fname, fsRemove(fname));

    if(fsUnmount(dirname) < 0) {
	perror("fsUnmount"); exit(1);
    }

    return 0;
}
