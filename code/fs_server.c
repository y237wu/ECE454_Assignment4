#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

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
        printf("fsServer <host folder name>\n");
        return -1;
    }

    char* folderName = (char*) argv[1];

    browse_dir( folderName, 0 );

    //launch server

    //receive calls from client and send back response accordingly

    return 0;
}
