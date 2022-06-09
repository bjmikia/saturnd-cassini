#ifndef SY5_PROJET_2021_2022_SATURND_OPTIONS_H
#define SY5_PROJET_2021_2022_SATURND_OPTIONS_H
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <poll.h>

#include "common.h"
#include "client-request.h"
#include "server-reply.h"

#define MAX_LEN 512

int create_tasks_files(char * taskdir);
int jobs_start(const char * taskspath);
void terminate( char* path_reply, int request);
int write_comm_time_infile(int requete,const char * dirtaskid);
int execute(char *argv[],const char * dirtaskid, char * buf);
int saturnd_create(const char * taskspath,const char* pathpipe,int requete,uint64_t bigTaskid);
int get_file_content (  char * dirpath ,char * filename,int fd);
int write_exitcode(const char * path_fichier, const char * path_reply, int requete);
int list_tasks (int request,char *reply_path,char  *dirpath );
int remove_task (int request,char *reply_path,char  *dirpath);
int write_stderr_stdout (int request,char *reply_path, char * dirpath ,char* filename);
#endif //SY5_PROJET_2021_2022_SATURND_OPTIONS_H
