#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>

#include "simplified_rpc/ece454rpc_types.h"

return_type ret;

//int fsMount( const char* localFolderName );
return_type fsMount(const int nparams, arg_type* argList)
{
    const char* localFolderName;
    //sockaddr_in clientAddr;

    if( nparams != 1 ) {
        ret.return_val = NULL;
        ret.return_size = 0;
        return ret;
    }

    localFolderName = (const char*) argList->arg_val;
    //clientAddr = *( (sockaddr_in*) argList->next->arg_value );

    printf("local folder name: %s\n", localFolderName);
    //printf("client address: %s:%u\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    ret.return_size = sizeof(int);
    ret.return_val = malloc( sizeof(int) );
    *( (int*)ret.return_val ) = 0;

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

    char* folderName = (char*) argv[1];

    //set up file system
    browse_dir( folderName, 0 );

    register_procedure("fsMount", 2, fsMount);

    //launch server
    launch_server();

    //receive calls from client and send back response accordingly

    return 0;
}
