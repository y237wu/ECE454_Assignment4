#include "ece454_fs.h"
#include "simplified_rpc/ece454rpc_types.h"

#include <string.h>
#include <limits.h>
#include <stdlib.h>

struct fsDirent dent;

struct fsDirList {
    FSDIR* value;
    struct fsDirList* next;
};

struct fileList {
    int fd;
    struct fileList* next;
};

struct fsSession {
    const char* serverIp;
    int serverPort;
    const char* localFolderName;
    int sessionId;

    struct fsSession* next;
    struct fsDirList* fsDirs;
    struct fileList* files;
};

struct fsSession* sessions = NULL;

void appendSession(struct fsSession* newSession)
{
    if( sessions == NULL ) {
        sessions = newSession;
        return;
    }

    struct fsSession** sessionPtr = &sessions;

    while( (*sessionPtr)->next != NULL ) {
        sessionPtr = &((*sessionPtr)->next);
    }

    (*sessionPtr)->next = newSession;
}

void removeSession(struct fsSession* removeSession)
{
    if( sessions == removeSession ) {
        sessions = sessions->next;
        return;
    }

    struct fsSession** sessionPtr = &sessions;
    struct fsSession** prevSessionPtr = NULL;

    while( *sessionPtr != NULL ) {
        if( *sessionPtr == removeSession )
            break;
        prevSessionPtr = sessionPtr;
        sessionPtr = &((*sessionPtr)->next);
    }

    if( *sessionPtr = NULL )
        return;

    (*prevSessionPtr)->next = *sessionPtr;
}

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

    //assuming pre-allocated folder name and server ip
    struct fsSession* sessionPtr = malloc(sizeof(struct fsSession));
    sessionPtr->serverIp = srvIpOrDomName;
    sessionPtr->serverPort = srvPort;
    sessionPtr->localFolderName = localFolderName;

    sessionPtr->sessionId = *( (int*) ret.return_val );
    free( ret.return_val );

    appendSession( sessionPtr );

    sessionPtr = sessions;

    return 0;
}

int fsUnmount(const char *localFolderName) {
    struct fsSession* sessionPtr = sessions;
    return_type ret;
    int retVal = -1;

    while( sessionPtr != NULL ) {
        if( strcmp( sessionPtr->localFolderName, localFolderName ) == 0 ) {
            retVal = 0;
            break;
        }
        sessionPtr = sessionPtr->next;
    }

    if( retVal != 0 || sessionPtr == NULL ) {
        printf("local folder does not exist\n");
        return -1;
    }

    ret = make_remote_call(sessionPtr->serverIp,
                           sessionPtr->serverPort,
                           "fsUnmount",
                           1,
                           sizeof(int),
                           &sessionPtr->sessionId);

    if( ret.return_size == 0 ) {
        printf("unmount remote call failed\n");
        errno = 1;
        return -1;
    }

    retVal = *( (int*) ret.return_val );
    free(ret.return_val);

    if( retVal != 0 ) {
        printf("unmount remote call failed\n");
        errno = retVal;
        return -1;
    }

    //delete session from session list
    removeSession( sessionPtr );
    free(sessionPtr);

    return retVal;
}

FSDIR* fsOpenDir(const char *folderName) {
    //search for session
    struct fsSession* sessionPtr = sessions;

    while( sessionPtr != NULL ) {
        if( strcmp(sessionPtr->localFolderName, folderName) == 0 ) {
            break;
        }
        sessionPtr = sessionPtr->next;
    }

    // session not found
    if( sessionPtr == NULL )
        return NULL;

    return_type ret = make_remote_call(sessionPtr->serverIp,
                                       sessionPtr->serverPort,
                                       "fsOpenDir",
                                       2,
                                       sizeof(int),
                                       &sessionPtr->sessionId,
                                       strlen(folderName) + 1,
                                       folderName);
    if( ret.return_size == 0 ) {
        printf("remote fsOpenDir error\n");
        return NULL;
    }

    FSDIR* retVal = *( (FSDIR**)ret.return_val );
    free(ret.return_val);

    //store open directory to the open session
    struct fsDirList* fsDirPtr = sessionPtr->fsDirs;
    while( fsDirPtr != NULL ) {
        fsDirPtr = fsDirPtr->next;
    }

    fsDirPtr = malloc( sizeof(struct fsDirList) );
    fsDirPtr->value = retVal;

    if(sessionPtr->fsDirs == NULL)
        sessionPtr->fsDirs = fsDirPtr;


    return retVal;
}

int fsCloseDir(FSDIR *folder) {
    struct fsSession* sessionPtr = sessions;
    struct fsDirList* prevfsDirPtr = NULL;
    struct fsDirList* fsDirPtr = NULL;
    bool found = false;

    //try to find the exisiting opened folder
    while( sessionPtr != NULL ) {
        fsDirPtr = sessionPtr->fsDirs;
        while( fsDirPtr != NULL ) {
            prevfsDirPtr = fsDirPtr;
            if( fsDirPtr->value == folder ) {
                found = true;
                break;
            }
            fsDirPtr = fsDirPtr->next;
        }
        if( found == true )
            break;
        sessionPtr = sessionPtr->next;
    }

    //folder not found
    if( found == false )
        return -1;

    return_type ret = make_remote_call(sessionPtr->serverIp,
                                       sessionPtr->serverPort,
                                       "fsCloseDir",
                                       2,
                                       sizeof(int),
                                       &sessionPtr->sessionId,
                                       sizeof(FSDIR*),
                                       &folder);

    if( ret.return_size == 0 ) {
        printf("remote fsCloseDir error\n");
        return -1;
    }

    int retVal = *( (int*)ret.return_val );
    free(ret.return_val);

    if( retVal != 0 ) {
        printf("CloseDir failed\n");
        errno = retVal;
        return -1;
    }
    printf("list operations\n");

    //remove the folder from the session folder list
    if( prevfsDirPtr == NULL ) {
        sessionPtr->fsDirs = fsDirPtr->next;
    } else {
        prevfsDirPtr->next = fsDirPtr->next;
    }
    free(fsDirPtr);

    return 0;
}

//not done yet
struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;

    struct fsSession* sessionPtr = sessions;
    struct fsDirList* fsDirPtr = NULL;
    bool found = false;

    //try to find the exisiting opened folder
    while( sessionPtr != NULL ) {
        fsDirPtr = sessionPtr->fsDirs;
        while( fsDirPtr != NULL ) {
            if( fsDirPtr->value == folder ) {
                found = true;
                break;
            }
            fsDirPtr = fsDirPtr->next;
        }
        if( found == true )
            break;
        sessionPtr = sessionPtr->next;
    }

    //folder not found
    if( found == false )
        return NULL;


    return_type ret = make_remote_call( sessionPtr->serverIp,
                                        sessionPtr->serverPort,
                                        "fsReadDir",
                                        2,
                                        sizeof(int),
                                        &sessionPtr->sessionId,
                                        sizeof(FSDIR*),
                                        &folder );

    if( ret.return_size == 0 ) {
        printf("remote fsReadDir error\n");
        return NULL;
    } else if( ret.return_size == sizeof(int) ) {
        int retVal = *( (int*) ret.return_val );
        free(ret.return_val);
        if( retVal == 0 )
            return NULL;
    }

    struct dirent *d = (struct dirent*) ret.return_val;

    memcpy(&(dent.entName), &(d->d_name), 256);
    free(ret.return_val);

    if( d == NULL )
        return NULL;

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

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

    //search for session
    struct fsSession* sessionPtr = sessions;

    while( sessionPtr != NULL ) {
        if( strncmp(sessionPtr->localFolderName, fname,
                    sizeof(sessionPtr->localFolderName) ) == 0 ) {
            break;
        }
        sessionPtr = sessionPtr->next;
    }

    // session not found
    if( sessionPtr == NULL ) {
        printf("fsOpen session not found\n");
        return -1;
    }

    int retVal;
    while(1) {
        return_type ret = make_remote_call(sessionPtr->serverIp,
                                           sessionPtr->serverPort,
                                           "fsOpenFile",
                                           3,
                                           sizeof(int),
                                           &sessionPtr->sessionId,
                                           strlen(fname) + 1,
                                           fname,
                                           sizeof(int),
                                           &flags);
        if( ret.return_size == 0 ) {
            printf("remote fsOpenFile error\n");
            return -1;
        }

        retVal = *( (int*) ret.return_val );
        free( ret.return_val );

        if( retVal == UNAVAIL ) {
            sleep(5);
        } else if( retVal > -1 ) {
            break;
        }
    }

    //store open directory to the open session
    struct fileList* filePtr = sessionPtr->files;
    while( filePtr != NULL ) {
        filePtr = filePtr->next;
    }

    filePtr = malloc( sizeof(struct fileList) );
    filePtr->fd = retVal;
    if( sessionPtr->files == NULL )
        sessionPtr->files = filePtr;

    return retVal;
}

int fsClose(int fd) {
    struct fsSession* sessionPtr = sessions;
    struct fileList* prevFilePtr = NULL;
    struct fileList* filePtr = NULL;
    bool found = false;

    //try to find the exisiting opened file
    while( sessionPtr != NULL ) {
        filePtr = sessionPtr->files;
        while( filePtr != NULL ) {
            prevFilePtr = filePtr;
            if( filePtr->fd == fd ) {
                found = true;
                break;
            }
            filePtr = filePtr->next;
        }
        if( found == true )
            break;
        sessionPtr = sessionPtr->next;
    }

    //file not found
    if( found == false )
        return -1;

    return_type ret = make_remote_call(sessionPtr->serverIp,
                                       sessionPtr->serverPort,
                                       "fsCloseFile",
                                       2,
                                       sizeof(int),
                                       &sessionPtr->sessionId,
                                       sizeof(int),
                                       &fd);
    if( ret.return_size == 0 ) {
        printf("remote fsCloseFile error\n");
        return -1;
    }

    int retVal = *( (int*)ret.return_val );
    free( ret.return_val );

    if( retVal != 0 ) {
        printf("CloseFile failed\n");
        errno = retVal;
        return -1;
    }

    //remove the file from the session file list
    if( prevFilePtr == NULL ) {
        sessionPtr->files = filePtr->next;
    } else {
        prevFilePtr->next = filePtr->next;
    }
    free(filePtr);

    return 0;
}

int fsRead(int fd, void *buf, const unsigned int count) {
    struct fsSession* sessionPtr = sessions;
    struct fileList* filePtr = NULL;
    bool found = false;

    //try to find the exisiting opened file
    while( sessionPtr != NULL ) {
        filePtr = sessionPtr->files;
        while( filePtr != NULL ) {
            if( filePtr->fd == fd ) {
                found = true;
                break;
            }
            filePtr = filePtr->next;
        }
        if( found == true )
            break;
        sessionPtr = sessionPtr->next;
    }

    //file not found
    if( found == false )
        return -1;

    return_type ret = make_remote_call(sessionPtr->serverIp,
                                       sessionPtr->serverPort,
                                       "fsRead",
                                       3,
                                       sizeof(int),
                                       &sessionPtr->sessionId,
                                       sizeof(int),
                                       &fd,
                                       sizeof(int),
                                       &count);
    if( ret.return_size == 0 ) {
        printf("remote fsReadFile error\n");
        return -1;
    }

    int retVal = ret.return_size;

    memcpy(buf, ret.return_val, count);
    free( ret.return_val );
    
    return retVal;
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    struct fsSession* sessionPtr = sessions;
    struct fileList* filePtr = NULL;
    bool found = false;

    //try to find the exisiting opened file
    while( sessionPtr != NULL ) {
        filePtr = sessionPtr->files;
        while( filePtr != NULL ) {
            if( filePtr->fd == fd ) {
                found = true;
                break;
            }
            filePtr = filePtr->next;
        }
        if( found == true )
            break;
        sessionPtr = sessionPtr->next;
    }

    //file not found
    if( found == false )
        return -1;

    return_type ret = make_remote_call(sessionPtr->serverIp,
                                       sessionPtr->serverPort,
                                       "fsWrite",
                                       4,
                                       sizeof(int),
                                       &sessionPtr->sessionId,
                                       sizeof(int), &fd,
                                       count, buf,
                                       sizeof(int), &count);
    if( ret.return_size == 0 ) {
        printf("remote fsWriteFile error\n");
        return -1;
    }

    int retVal = *( (int*) ret.return_val );
    free( ret.return_val );

    if( retVal < 0 ) {
        printf("unmount remote call failed\n");
        errno = retVal;
        return -1;
    }

    return retVal;
}

int fsRemove(const char *name) {
    struct fsSession* sessionPtr = sessions;
    bool success = false;

    int nameLen = strlen(name)+1;

    //try to find the exisiting opened file
    while( sessionPtr != NULL ) {
        return_type ret = make_remote_call(sessionPtr->serverIp,
                                           sessionPtr->serverPort,
                                           "fsRemove",
                                           2,
                                           sizeof(int),
                                           &sessionPtr->sessionId,
                                           nameLen,
                                           name);
        if( ret.return_size == 0 ) {
            printf("remote fsRemoveFile error\n");
            return -1;
        }

        int retVal = *( (int*) ret.return_val );
        free(ret.return_val);

        if( retVal != 0 ) {
            printf("remove file remote call failed\n");
            errno = retVal;
        } else {
            success = true;
        }

        if( success == true )
            return 0;

        sessionPtr = sessionPtr->next;
    }

    //remove failed
    return -1;
}
