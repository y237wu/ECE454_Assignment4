#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* A fake server that does nothing useful other than illustrate how
 * the unpacking for A4 of ECE454 should work.
 */

void usage(char *pname) {
    fprintf(stderr, "usage: %s <directory to serve>\n", pname);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
	usage(argv[0]); return 1;
    }

    struct stat buf;
    if(stat(argv[1], &buf) < 0) {
	perror("stat");
	usage(argv[0]);
	return 1;
    }

    if(!S_ISDIR(buf.st_mode)) {
	fprintf(stderr, "error: %s does not appear to be a directory.\n", argv[1]);
	usage(argv[0]);
	return 1;
    }

    /* Basic checks ok. Print out a fake ip address & port &
     * block forever.
     */
    printf("1.2.3.4 1000\n"); fflush(stdout);
    while(1) {
	char c;
	scanf("%c", &c);
    }

    return 0;
}
