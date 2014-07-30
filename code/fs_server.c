#include "ece454_fs.h"
#include "simplified_rpc/ece454rpc_types.h"

#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

return_type ret;

int g_sessionId = 0;
const char* serverFolderName;

struct mountSession {
    int sessionId;
    char* clientFolderName;

    struct mountSession* next;
};

struct folderLock {
    int sessionId;
    DIR* dir;
    char dname[PATH_MAX];
    struct folderLock* next;
};

struct fileLock {
    int sessionId;
    int fd;
    int flags;
    char fname[PATH_MAX];
    struct fileLock* next;
};

struct mountSession* sessions = NULL;
struct folderLock* folderLocks = NULL;
struct fileLock* fileLocks = NULL;

void appendMountSession(struct mountSession* newSession)
{
    if( sessions == NULL ) {
        sessions = newSession;
        return;
    }

    struct mountSession** sessionPtr = &sessions;

    while( (*sessionPtr)->next != NULL ) {
        sessionPtr = &((*sessionPtr)->next);
    }

    (*sessionPtr)->next = newSession;
}

void removeMountSession(struct mountSession* removeSession)
{
    if( sessions == removeSession ) {
        sessions = sessions->next;
        return;
    }

    struct mountSession** sessionPtr = &sessions;
    struct mountSession** prevSessionPtr = NULL;

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

void appendFolderLock(struct folderLock* newItem)
{
    if( folderLocks == NULL ) {
        folderLocks = newItem;
        return;
    }

    struct folderLock** ptr = &folderLocks;

    while( (*ptr)->next != NULL ) {
        ptr = &((*ptr)->next);
    }

    (*ptr)->next = newItem;

    printf("after appendFoldereLock\n");
    struct folderLock* fPtr = folderLocks;
    while( fPtr!= NULL ) {
        printf("dir=%d\n", fPtr->dir);
        fPtr = fPtr->next;
    }
}

void removeFolderLock(struct folderLock* removeItem)
{
    if( folderLocks == removeItem ) {
        folderLocks = folderLocks->next;
        return;
    }

    struct folderLock** ptr = &folderLocks;
    struct folderLock** prevPtr = NULL;

    while( *ptr != NULL ) {
        if( *ptr == removeItem )
            break;
        prevPtr = ptr;
        ptr = &((*ptr)->next);
    }

    if( *ptr = NULL )
        return;

    (*prevPtr)->next = *ptr;
}

void appendFileLock(struct fileLock* newItem)
{
    if( fileLocks == NULL ) {
        fileLocks = newItem;
        return;
    }

    struct fileLock** ptr = &fileLocks;

    while( (*ptr)->next != NULL ) {
        ptr = &((*ptr)->next);
    }

    (*ptr)->next = newItem;

    printf("after appendFileLock\n");
    struct fileLock* fPtr = fileLocks;
    while( fPtr!= NULL ) {
        printf("fd=%d\n", fPtr->fd);
        fPtr = fPtr->next;
    }
}

void removeFileLock(struct fileLock* removeItem)
{
    if( fileLocks == removeItem ) {
        fileLocks = fileLocks->next;
        return;
    }

    struct fileLock** ptr = &fileLocks;
    struct fileLock** prevPtr = NULL;

    while( *ptr != NULL ) {
        if( *ptr == removeItem )
            break;
        prevPtr = ptr;
        ptr = &((*ptr)->next);
    }

    if( *ptr = NULL )
        return;

    (*prevPtr)->next = *ptr;
}

int findSession(int clientSessionId, struct mountSession** sessionPtr)
{
    while( *sessionPtr != NULL ) {
        if( (*sessionPtr)->sessionId == clientSessionId ) {
            break;
        }
        *sessionPtr = (*sessionPtr)->next;
    }

    //session not found
    if( *sessionPtr == NULL ) {
        return -1;
    }

    return 0;
}

char* replaceClientLocalFolder(const char* clientDir,
        struct mountSession* session)
{
    int clientDirLen = strlen(clientDir);
    int clientFolderLen = strlen(session->clientFolderName);
    int serverDirLen = clientDirLen - clientFolderLen
        + strlen(serverFolderName);

    char* serverDir = malloc(serverDirLen + 1);
    memset(serverDir, 0, serverDirLen+1);

    strcat(serverDir, serverFolderName);
    strcat(serverDir, clientDir + clientFolderLen);

    return serverDir;
}

return_type rpc_fsMount(const int nparams, arg_type* argList)
{

    if( nparams != 1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    const char* clientFolderName = (const char*) argList->arg_val;

    struct mountSession* sessionPtr = sessions;

    while( sessionPtr != NULL ) {
        //puplicate local folder name, rejected
        if( strcmp(sessionPtr->clientFolderName, clientFolderName) == 0 ) {
            ret.return_val = NULL;
            ret.return_size = 0;
            return ret;
        }
        sessionPtr = sessionPtr->next;
    }

    printf("local folder name: %s\n", clientFolderName);

    sessionPtr = malloc(sizeof(struct mountSession));
    sessionPtr->clientFolderName = malloc(strlen(clientFolderName)+1);
    strcpy(sessionPtr->clientFolderName, clientFolderName);
    sessionPtr->next = NULL;

    sessionPtr->sessionId = g_sessionId++;

    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = sessionPtr->sessionId;

    appendMountSession(sessionPtr);
    sessionPtr = sessions;

    return ret;
}

return_type rpc_fsUnmount(const int nparams, arg_type* argList)
{
    if( nparams != 1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    int clientSessionId = *( (int*) argList->arg_val );

    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //remove session from list
    removeMountSession(sessionPtr);
    free(sessionPtr->clientFolderName);
    free(sessionPtr);

    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = 0;

    return ret;
}

return_type rpc_fsOpenDir(const int nparams, arg_type* argList)
{
    if( nparams != 2 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    int clientSessionId = *( (int*) argList->arg_val );
    char* requestDir = (char*) argList->next->arg_val;

    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //open server directory and return DIR
    const char* serverFolder = replaceClientLocalFolder(requestDir,
                                                    sessionPtr);
    printf("serverFolder: %s\n", serverFolder);
    //lock the opened folder
    struct folderLock* lockPtr = folderLocks;

    //do not create another lock when folder is already opened
    //by the session
    bool opened = false;
    while( lockPtr != NULL ) {
        if( lockPtr->sessionId == sessionPtr->sessionId &&
                strcmp(lockPtr->dname, serverFolder) == 0 ) {
            opened = true;
            break;
        }
        lockPtr = lockPtr->next;
    }

    if( opened == false ) {
        DIR* dir = opendir(serverFolder);

        //error opening directory
        if( dir == NULL ) {
            ret.return_size = 0;
            ret.return_val = NULL;
            return ret;
        }

        lockPtr = malloc(sizeof(struct folderLock));
        lockPtr->sessionId = sessionPtr->sessionId;
        lockPtr->dir = dir;
        strcpy(lockPtr->dname, serverFolder);
        lockPtr->next = NULL;

        appendFolderLock(lockPtr);
    }

    ret.return_size = sizeof(DIR*);
    ret.return_val = malloc( sizeof(DIR*) );
    memcpy(ret.return_val, &lockPtr->dir, sizeof(DIR*));

    return ret;
}

return_type rpc_fsCloseDir(const int nparams, arg_type* argList)
{
    if( nparams != 2 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    DIR* requestDir = *( (DIR**) argList->next->arg_val );

    //FIND SESSION
    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //find the opened folder
    struct folderLock* lockPtr = folderLocks;
    while( lockPtr != NULL ) {
        if( lockPtr->sessionId == sessionPtr->sessionId &&
                lockPtr->dir == requestDir ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    //open folder not found
    if( lockPtr == NULL ) {
        printf("closeDir folder not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //discard lock even if closedir failed because the dir* itself is wrong
    removeFolderLock(lockPtr);
    free(lockPtr);

    //see if other sessions still has the same file opened
    lockPtr = folderLocks;
    while( lockPtr != NULL ) {
        if( lockPtr->dir == requestDir ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    if( lockPtr == NULL ) {
        //close directory
        int closeDirRet =  closedir(requestDir);
        //close dir failed and we want to return errorno
        //we will think about this later
        if( closeDirRet != 0 ) {
            ret.return_val = NULL;
            ret.return_size = 0;
            return ret;
        }
    }

    //CONSTRUCT RETURN TYPE
    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = 0;

    return ret;
}

return_type rpc_fsReadDir(const int nparams, arg_type* argList)
{
    if( nparams != 2 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    DIR* requestDir = *( (DIR**) argList->next->arg_val );

    //FIND SESSION
    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        printf("readDir session not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //find opened folder for the session
    struct folderLock* lockPtr = folderLocks;
    while( lockPtr != NULL ) {
        if( lockPtr->sessionId == sessionPtr->sessionId &&
                lockPtr->dir == requestDir ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    //open folder not found
    if( lockPtr == NULL ) {
        printf("readDir folder lock not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //read directory
    int initErrno = errno;
    struct dirent* ent =  readdir(requestDir);

    if( ent == NULL  && errno == initErrno) {
        //end of folder, returning null
        ret.return_size = sizeof( int );
        ret.return_val = malloc( sizeof(int) );
        memset( ret.return_val, 0, sizeof(int) );
    } else if( ent != NULL ) {
        //return the directory entity
        ret.return_size = sizeof( struct dirent );
        ret.return_val = malloc( sizeof(struct dirent) );
        memcpy( ret.return_val, ent, sizeof(struct dirent) );
    } else {
        printf("readDir folder failed\n");
        ret.return_val = NULL;
        ret.return_size = 0;
    }

    return ret;
}

return_type rpc_fsOpenFile(const int nparams, arg_type* argList)
{
    if( nparams != 3 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    int clientSessionId = *( (int*) argList->arg_val );
    char* requestFile = (char*) argList->next->arg_val;
    int flags = *( (int*) argList->next->next->arg_val );

    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        printf("fsOpen session not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //open server directory and return DIR
    char* serverFile = replaceClientLocalFolder(requestFile, sessionPtr);

    printf("fsOpenFile serverFile %s\n", serverFile);
    //check if the file is locked or not
    struct fileLock* lockPtr = fileLocks;
    int fd = -1;

    while( lockPtr != NULL ) {
        //a lock on the file exists
        if( strcmp(lockPtr->fname, serverFile) == 0 ) {
            if( lockPtr->sessionId == clientSessionId && lockPtr->flags == flags ) {
                //file locked by client itself in the same mode
                //not sure if this is valid or not, but let just do nothing
                fd = lockPtr->fd;
            } else {
                if( flags == O_RDONLY && lockPtr->flags == O_RDONLY ) {
                    //file is aquired by another client in read mode
                    //it is fine, simply add another lock to the same file
                    //but not reopen the file
                    fd = lockPtr->fd;

                    lockPtr = malloc(sizeof(struct fileLock));
                    lockPtr->sessionId = clientSessionId;
                    lockPtr->fd = fd;
                    lockPtr->flags = flags;
                    strcpy(lockPtr->fname, serverFile);
                    lockPtr->next = NULL;

                    appendFileLock(lockPtr);
                } else {
                    //file is locked and not available
                    ret.return_size = sizeof(int);
                    ret.return_val = malloc( sizeof(int) );
                    *( (int*)ret.return_val ) = UNAVAIL;

                    return ret;
                }
            }
            break;
        }
        lockPtr = lockPtr->next;
    }

    if( lockPtr == NULL ) {
        //file is not locked. open file and lock it
        fd = open(serverFile, flags);

        //error opening file
        if( fd == -1 ) {
            ret.return_size = 0;
            ret.return_val = NULL;
            return ret;
        }

        lockPtr = malloc(sizeof(struct fileLock));
        lockPtr->sessionId = sessionPtr->sessionId;
        lockPtr->fd = fd;
        lockPtr->flags = flags;
        strcpy(lockPtr->fname, serverFile);
        lockPtr->next = NULL;

        appendFileLock(lockPtr);
    }

    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = fd;

    return ret;
}

return_type rpc_fsCloseFile(const int nparams, arg_type* argList)
{
    if( nparams != 2 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    int requestFile = *( (int*) argList->next->arg_val );

    //FIND SESSION
    struct mountSession* sessionPtr = sessions;
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //find the opened file
    struct fileLock* lockPtr = fileLocks;

    while( lockPtr != NULL ) {
        if( lockPtr->sessionId == sessionPtr->sessionId &&
                lockPtr->fd == requestFile ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    //open file not found
    if( lockPtr == NULL ) {
        printf("fsCloseFile file not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //discard lock even if close failed because the fd itself is wrong
    removeFileLock(lockPtr);
    free(lockPtr);

    //check if other sessions still has the file opened
    lockPtr = fileLocks;
    while(lockPtr != NULL) {
        if( lockPtr->fd == requestFile )
            break;
        lockPtr = lockPtr->next;
    }

    if( lockPtr == NULL ) {
        //close file and remove file lock
        int closeFileRet =  close(requestFile);

        //close failed and we want to return errorno
        //we will think about this later
        if( closeFileRet != 0 ) {
            ret.return_val = NULL;
            ret.return_size = 0;
            return ret;
        }
    }

    //returining 0 on success
    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = 0;

    return ret;
}

return_type rpc_fsRead(const int nparams, arg_type* argList)
{
    if( nparams != 3 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    int requestFile = *( (int*) argList->next->arg_val );
    int readCount = *( (int*) argList->next->next->arg_val );


    //FIND SESSION
    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //FUNCTION MAIN BODY
    //find folder lock
    struct fileLock* lockPtr = fileLocks;

    while( lockPtr != NULL ) {
        printf("fsRead fileLocks != NULL\n");
        if( lockPtr->sessionId == clientSessionId
                && lockPtr->fd == requestFile 
                && lockPtr->flags == O_RDONLY ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    //lock not found
    if( lockPtr == NULL ) {
        printf("fsRead file lock not found\n");
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    void* buffer = malloc(readCount);
    memset(buffer, 0, readCount);
    int readSuccess = read( requestFile, buffer, readCount );

    if( readSuccess == -1 ) {
        free(buffer);
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //CONSTRUCT RETURN TYPE
    ret.return_size = readSuccess;
    ret.return_val = malloc( readSuccess );
    memcpy(ret.return_val, buffer, readSuccess);

    return ret;
}

return_type rpc_fsWrite(const int nparams, arg_type* argList)
{
    if( nparams != 4 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    int requestFile = *( (int*) argList->next->arg_val );
    void* writeBuffer = argList->next->next->arg_val;
    int writeCount = *( (int*) argList->next->next->next->arg_val );

    //FIND SESSION
    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //FUNCTION MAIN BODY
    //find file lock
    struct fileLock* lockPtr = fileLocks;

    while( lockPtr != NULL ) {
        if( lockPtr->sessionId == clientSessionId
                && lockPtr->fd == requestFile 
                && lockPtr->flags == O_WRONLY | O_CREAT ) {
            break;
        }
        lockPtr = lockPtr->next;
    }

    //lock not found
    if( lockPtr == NULL ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    int writeSuccess = write( requestFile, writeBuffer, writeCount );
    printf("writeSuccess %d requestFile: %d writeCount %d\n", writeSuccess, requestFile, writeCount);

    if( writeSuccess == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //CONSTRUCT RETURN TYPE
    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*) ret.return_val) = writeSuccess;

    return ret;
}

return_type rpc_fsRemove(const int nparams, arg_type* argList)
{
    if( nparams != 2 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //INITIALIZE PARAMS TO LOCAL VARIABLES
    int clientSessionId = *( (int*) argList->arg_val );
    char* requestDir = (char*) argList->next->arg_val;

    //FIND SESSION
    struct mountSession* sessionPtr = sessions;

    //session not found
    if( findSession(clientSessionId, &sessionPtr) == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //FUNCTION MAIN BODY
    const char* serverDir = replaceClientLocalFolder(requestDir,
                                                    sessionPtr);
    //find folder lock
    struct folderLock* folderLockPtr = folderLocks;
    struct fileLock* fileLockPtr = fileLocks;

    while( folderLockPtr != NULL ) {
        if( strcmp(folderLockPtr->dname, serverDir) == 0 ) {
            break;
        }
        folderLockPtr = folderLockPtr->next;
    }

    while( fileLockPtr != NULL ) {
        if( strcmp(fileLockPtr->fname, serverDir) == 0 ) {
            break;
        }
        fileLockPtr = fileLockPtr->next;
    }

    //if either the folder or the file is locked
    if( folderLockPtr != NULL || fileLockPtr != NULL ) {
        ret.return_size = sizeof(int);
        ret.return_val = malloc(sizeof(int));
        *( (int*)ret.return_val ) = UNAVAIL;
        return ret;
    }

    int removeSuccess = remove( serverDir );

    if( removeSuccess == -1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    //CONSTRUCT RETURN TYPE
    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    (*(int*) ret.return_val) = removeSuccess;

    return ret;
}

void browse_dir(const char* dirName, int depth)
{
    DIR* dir = opendir(dirName);
    
    if( dir == NULL ) {
        if( errno == ENOENT ) {
            printf("folder does not exists\n");
        } else {
            printf( "error: %s\n", strerror(errno) );
        }
        return;
    }

    while( dir != NULL ) {
        struct dirent* entry;
        entry = readdir(dir);

        if( entry != NULL ) {
            int i;
            for(i = 0; i < depth; i++)
                printf("  ");
            printf("%s\n", entry->d_name);

            if( entry->d_type == DT_DIR &&
                    strcmp( entry->d_name, "..") != 0 &&
                    strcmp( entry->d_name, "." ) != 0 )
            {
                char path[PATH_MAX];
                snprintf(path, PATH_MAX, "%s/%s", dirName, entry->d_name);
                browse_dir( path, depth+1);
            }
        } else {
            if( errno != 0 ) {
                printf( "error: %s\n", strerror(errno) );
            }
            break;
        }
    }
    closedir(dir);
}

int main(int argc, void* argv[])
{
    //parse input
    if( argc != 2 ) {
        printf("Server <host folder name>\n");
        return -1;
    }

    serverFolderName = (char*) argv[1];

    register_procedure("fsMount", 1, rpc_fsMount);
    register_procedure("fsUnmount", 1, rpc_fsUnmount);
    register_procedure("fsOpenDir", 2, rpc_fsOpenDir);
    register_procedure("fsCloseDir", 2, rpc_fsCloseDir);
    register_procedure("fsReadDir", 2, rpc_fsReadDir);
    register_procedure("fsOpenFile", 3, rpc_fsOpenFile);
    register_procedure("fsCloseFile", 2, rpc_fsCloseFile);
    register_procedure("fsRead", 3, rpc_fsRead);
    register_procedure("fsWrite", 4, rpc_fsWrite);
    register_procedure("fsRemove", 2, rpc_fsRemove);

    //launch server
    //receive calls from client and send back response accordingly
    launch_server();

    return 0;
}
