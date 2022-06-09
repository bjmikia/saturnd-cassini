#include "common.h"

/** Construit une structure timing avec les arguments passés en paramètre , renvoie NULL en cas d'erreur
* @param min
* @param hours
* @param days
* @return struct timing
*/
struct timing * init_timing( char * min , char * hours, char * days){
    struct timing * time = malloc(sizeof(struct timing));
    if(time == NULL){
        perror("init timing: struct timing null");
        return NULL;
    }
    int result_time  = timing_from_strings(time, min, hours, days);
    if(result_time == -1){
        perror("init timing :timing from string");
        free(time);
        return NULL;
    }
    return time;
}

/** Fonction qui concatène les trois chaines passées en paramètre
* @param str1
* @param str2
* @param str3
* @return un buffer contenant la nouvelle chaine
*/
char * concat_strings(const char * str1, const char * str2, const char * str3) {
    char* buf = malloc(strlen(str1) + strlen(str2) + strlen (str3) + 1);
    if(buf == NULL){
        perror("concat strings : buf null");
        return NULL;
    }
    sprintf(buf,"%s%s%s", str1,str2,str3);
    return buf;
}

/** Ecrit dans le descripteur fd la structure string composée de
* la longueur de la chaine str_sent et de str_sent elle-même
* @param fd : descripteur d'un tube
* @param str_sent : chaine à envoyer
* @return 0 si la chaine a été envoyé dans le tube, 1 en cas
* d'erreur
*/
int send_string(char* buf_cur,int * buf_s, int argc, char ** argv , int i){
    string * argument = NULL;

    while (i<argc){
        char * str_sent = argv[i];
        argument = malloc(sizeof(string));
        if(argument == NULL){
            perror("send string: argument null");
            return 1;
        }
        //conversion big-endien
        argument->l = htobe32(strlen(str_sent));
        argument->str = str_sent;

        memmove(buf_cur,&(argument -> l),sizeof(argument -> l));
        buf_cur = buf_cur + sizeof(argument -> l);
        *buf_s = *buf_s + sizeof(argument -> l);


        memmove(buf_cur,argument -> str,strlen(str_sent));
        buf_cur = buf_cur + strlen(str_sent);
        *buf_s = *buf_s + strlen(str_sent);

        i++;
    }

    free(argument);
    return 0;
}

/** Ecrit dans le descripteur fd le nombre d'argument de commandline, puis
 * écrit les structures associées à ces arguments grâce à send_string
 * @param fd :descripteur d'un tube
 * @param argc : nombre total d'argument du main
 * @param argv : argument du main
 * @param first_command : indice du premier argument de argv qui n'est pas
 * une option de cassini, obtenu grace à optind
 * @return 0 si la commandline à été envoyé, 1 en cas d'erreur
*/
int send_commandline( char* buf_cur,int * buf_s, int argc, char ** argv , int first_command){
    uint32_t nb_arg = argc-first_command;
    if(nb_arg < 1)return 1;

    nb_arg = htobe32(nb_arg);
    memmove(buf_cur,&nb_arg,sizeof(nb_arg));
    buf_cur = buf_cur + sizeof(nb_arg);
    *buf_s = *buf_s + sizeof(nb_arg);

    int i = first_command;
    send_string(buf_cur,buf_s,argc,argv ,i) ;

    return 0;
}

