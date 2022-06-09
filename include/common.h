#ifndef SY5_PROJET_2021_2022_COMMON_H
#define SY5_PROJET_2021_2022_COMMON_H
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <bits/stdint-uintn.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>

#include "stringb.h"
#include "commandline.h"
#include "timing-text-io.h"

int send_string(char* buf_cur,int * buf_s, int argc, char ** argv , int i);
int send_commandline(char * buf_cur,int * buf_s, int argc, char ** argv , int first_command);
char * concat_strings(const char * str1, const char * str2, const char * str3);
struct timing * init_timing( char * min , char * hours, char * days);

#endif //SY5_PROJET_2021_2022_COMMON_H
