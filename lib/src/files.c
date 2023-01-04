
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/files.h"

void write_files_recursively(char *filename, char * dirname, long nfiles)
{
    DIR *tp = opendir(dirname);
    
    if(tp == NULL) {
        perror("Directory: ");
        return;
    }

    struct dirent* pt;
    struct stat st;

    char path[108];

    if (!(lstat(dirname, &st)>=0)) {
        errno = ENOTDIR;
        perror("Directory: ");
        return;
    } 

    if(nfiles == 0) nfiles = INT_MAX;

    while((pt = readdir(tp))!=NULL && nfiles >= 0) 
	{
        // . o ..
		if (!strcmp(pt->d_name, ".")||!strcmp(pt->d_name, "..")) continue;
		
        strcpy(path, dirname);
		
        // Creo il path
        if (path[strlen(path)-1]!='/') strcat(path, "/"); 
        
        strcat(path, pt->d_name);
		
        // Errore :
        if (!(lstat(path, &st)>=0)) {
            perror("lstat:");
            continue;
        }
		
        // E' una directory :
        if (S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Directory trovata: %s\n", pt->d_name);
            write_files_recursively(".", path, nfiles);
        }

        // E' un file :
        else if (S_ISREG(st.st_mode)) {
            
            fprintf(stderr, "File trovato: [%s] \n", path);

            
        }
    }
    closedir(tp);

    return;
}

