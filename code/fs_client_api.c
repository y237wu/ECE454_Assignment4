#include "ece454_fs.h"
#include "simplified_rpc/ece454rpc_types.h"

#include <string.h>

struct fsDirent dent;

const char* serverIp;
const unsigned int serverPort;
const char* localFolderName;

int fsMount(const char *srvIpOrDomName,
            const unsigned int srvPort,
            const char *localFolderName)
{
    return_type ret;
    ret = make_remote_call(srvIpOrDomName,
                           srvPort,
                           "fsMount",
                           1,
                           strlen(localFolderName) + 1,
                           localFolderName);
    if( ret.return_size == 0 ) {
        printf("mount error\n");
        errno = 1;
        return -1;
    }

    int retVal = *( (int*)ret.return_val );

    if( retVal != 0 ) {
        printf("mount error\n");
        errno = retVal;
        return -1;
    }

    return 0;
}

int fsUnmount(const char *localFolderName) {
    return 0;
}

FSDIR* fsOpenDir(const char *folderName) {
    return(opendir(folderName));
}

int fsCloseDir(FSDIR *folder) {
    return(closedir(folder));
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    struct dirent *d = readdir(folder);

    if(d == NULL) {
	if(errno == initErrno) errno = 0;
	return NULL;
    }

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

    memcpy(&(dent.entName), &(d->d_name), 256);
    return &dent;
}

int fsOpen(const char *fname, int mode) {
    int flags = -1;

    if(mode == 0) {
	flags = O_RDONLY;
    }
    else if(mode == 1) {
	flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));
}

int fsClose(int fd) {
    return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    return(write(fd, buf, (size_t)count)); 
}

int fsRemove(const char *name) {
    return(remove(name));
}
