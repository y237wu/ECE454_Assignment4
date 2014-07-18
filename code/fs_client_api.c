#include "ece454_fs.h"
#include "simplified_rpc/ece454rpc_types.h"

#include <string.h>
#include <limits.h>
#include <stdlib.h>

struct fsDirent dent;

struct fsSession {
    char* serverIp;
    unsigned int serverPort;
    char* localFolderName;
    char* remoteFolderName;
    int sessionId;
} session;

//assuming one session right now, if we need more, than we add list
//list<fsSession*> fsSessionList;

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

    session.serverIp = srvIpOrDomName;
    session.serverPort = srvPort;
    session.localFolderName = localFolderName;

    session.sessionId = *( (int*) ret.return_val );

    session.remoteFolderName = malloc( ret.return_size - sizeof(int) );
    strcpy(session.remoteFolderName, ret.return_val + sizeof(int*)); 

    /* if we need to support concurrent mount in one process
    session = malloc( sizeof(fsSession) );
    //assuming pre-allocated folder name and server ip
    session->serverIp = srvIporDomName;
    session->serverPort = srvPort;
    session->localFolderName = localFolderName;

    session->remoteFolderName = malloc( ret.return_size );
    strcpy(session->remoteFolderName, ret.return_val, ret.return_size);

    fsSessionList.push_back(session);
    */

    return 0;
}

int fsUnmount(const char *localFolderName) {

    /* if we need concurrent mount in one process
    list<fsSession*>::iterator iter;
    return_type ret;
    int retVal = -1;

    for(iter = fsSessionList.begin(); iter != fsSessionList.end(); iter++) {
        if( strcmp( (*iter)->localFolderName, localFolderName ) == 0 ) {
            retVal = 0;
            break;
        }
    }

    if( retVal != 0 ) {
        printf("local folder does not exist\n");
        return -1;
    }

    ret = make_remote_call(iter->serverIp,
                           iter->serverPort,
                           "Unmount",
                           1,
                           strlen(iter->localFolderName) + 1,
                           iter->localFolderName);

    if( ret->return_size == 0 ) {
        printf("unmount remote call failed\n");
        errno = 1;
        return -1;
    }

    retVal = *( (int*) ret->return_val );

    if( retVal != 0 ) {
        printf("unmount remote call failed\n");
        errno = retVal;
        return -1;
    }

    free( (*iter)->remoteFolderName );
    free(*iter);
    fsSessionList.erase(iter);

    return retVal;
    */
    return_type ret;

    ret = make_remote_call(session.serverIp,
                           session.serverPort,
                           "Unmount",
                           1,
                           sizeof(int),
                           session.sessionId);

    if( ret.return_size == 0 ) {
        printf("unmount remote call failed\n");
        errno = 1;
        return -1;
    }

    int retVal = *( (int*) ret.return_val );

    if( retVal != 0 ) {
        printf("unmount remote call failed\n");
        errno = retVal;
        return -1;
    }

    free( session.remoteFolderName );

    return retVal;
}

FSDIR* fsOpenDir(const char *folderName) {
    int tmp = strcmp(folderName, session.localFolderName);
    char remoteDir[PATH_MAX];

    strcat(remoteDir, session.remoteFolderName);
    strcat(remoteDir, folderName + tmp);
    printf("remoteDire: %s\n", remoteDir);

    return_type ret = make_remote_call(session.serverIp,
                                       session.serverPort,
                                       "OpenDir",
                                       2,
                                       sizeof(int),
                                       session.sessionId,
                                       strlen(remoteDir) + 1,
                                       remoteDir);
    if( ret.return_size == 0 ) {
        printf("remote fsOpenDir error\n");
        return NULL;
    }

    return (FSDIR*) ret.return_val;
}

int fsCloseDir(FSDIR *folder) {
    return_type ret = make_remote_call(session.serverIp,
                                       session.serverPort,
                                       "CloseDir",
                                       2,
                                       sizeof(int),
                                       session.sessionId,
                                       sizeof(FSDIR*),
                                       folder);
    if( ret.return_size == 0 ) {
        printf("remote fsCloseDir error\n");
        return -1;
    }

    int retVal = *( (int*)ret.return_val );
    if( retVal != 0 ) {
        printf("CloseDir failed\n");
        errno = retVal;
        return -1;
    }

    return 0;
}

//not done yet
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
