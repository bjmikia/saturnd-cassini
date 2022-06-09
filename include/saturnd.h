#ifndef SY5_PROJET_2021_2022_SATURND_H
#define SY5_PROJET_2021_2022_SATURND_H
#define _GNU_SOURCE 1
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <bits/stdint-uintn.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>


#include "saturnd_options.h"



int read_request_write_reply (void);
void init_deamon(void);

#endif //SY5_PROJET_2021_2022_SATURND_H
