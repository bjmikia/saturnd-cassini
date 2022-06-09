#include "../include/saturnd_options.h"

void terminate( char* path_reply, int request){
    close(request);
    uint16_t error = htobe16(SERVER_REPLY_OK);
    int reply = open(path_reply, O_WRONLY);
    write(reply, &error , sizeof(error));
}

