#ifndef SY5_PROJET_2021_2022_COMMANDLINE_H
#define SY5_PROJET_2021_2022_COMMANDLINE_H

#include <stdint.h>

typedef struct commandline commandline;
struct commandline{
    uint32_t argc;
    string* argv;
};

#endif //SY5_PROJET_2021_2022_COMMANDLINE_H
