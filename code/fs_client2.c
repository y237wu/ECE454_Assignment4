#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

int main(int argc, char *argv[]) {
    if(argc < 4) {
	fprintf(stderr, "usage: %s <srv-ip/name> <srv-port> <local dir name>\n", argv[0]);
	exit(1);
    }

    char *dirname = argv[3];
    printf("fsMount(1): %d\n", fsMount(argv[1], atoi(argv[2]), "/1"));
    printf("fsMount(2): %d\n", fsMount(argv[1], atoi(argv[2]), "/2"));
    printf("fsMount(3): %d\n", fsMount(argv[1], atoi(argv[2]), "/3"));

    FSDIR* dir1 = fsOpenDir("/1");
    FSDIR* dir2 = fsOpenDir("/2");
    FSDIR* dir3 = fsOpenDir("/3");

    int fd1 = fsOpen("/1/helper.c",0);
    int fd2 = fsOpen("/2/helper.c",0);
    int fd3 = fsOpen("/3/helper.c",0);

    fsClose(fd1);
    fsClose(fd2);
    fsClose(fd3);

    fsCloseDir(dir1);
    fsCloseDir(dir2);
    fsCloseDir(dir3);

    printf("fsUnmount(1): %d\n", fsUnmount("/1"));
    printf("fsUnmount(2): %d\n", fsUnmount("/2"));
    printf("fsUn,ount(3): %d\n", fsUnmount("/3"));

    return 0;
}
