#ifndef CASSINI_H
#define CASSINI_H
#define _GNU_SOURCE 1
#define MAX_SIZE 512


#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <bits/stdint-uintn.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "client-request.h"
#include "server-reply.h"
#include "common.h"

int write_request_cassini (uint16_t operation, uint64_t taskid, struct timing* time, char* path, int argc, char** argv, int indice_arg);
int show_reply_daemon (char* path , uint16_t operation);


// Helper functions
int show_reply_LS ( int reponse);
int show_reply_TX (int reponse, uint16_t reptype);
int show_reply_SO_SE ( int reponse ,uint16_t reptype) ;


#endif // CASSINI
