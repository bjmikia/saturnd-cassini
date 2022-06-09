#include "cassini.h"

const char usage_info[] = "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                       MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv[]) {
    errno = 0;

    char * minutes_str = "*";
    char * hours_str = "*";
    char * daysofweek_str = "*";
    char * pipes_directory = NULL;
    char * directory_basename = NULL;

    uint16_t operation = CLIENT_REQUEST_LIST_TASKS;
    uint64_t taskid =0;

    int opt;
    char * strtoull_endp;
    while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
        switch (opt) {
            case 'm':
                minutes_str = optarg;
                break;
            case 'H':
                hours_str = optarg;
                break;
            case 'd':
                daysofweek_str = optarg;
                break;
            case 'p':
                pipes_directory = strdup(optarg);
                if (pipes_directory == NULL) goto error;
                break;
            case 'l':
                operation = CLIENT_REQUEST_LIST_TASKS;
                break;
            case 'c':
                operation = CLIENT_REQUEST_CREATE_TASK;
                break;
            case 'q':
                operation = CLIENT_REQUEST_TERMINATE;
                break;
            case 'r':
                operation = CLIENT_REQUEST_REMOVE_TASK;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'x':
                operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'o':
                operation = CLIENT_REQUEST_GET_STDOUT;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'e':
                operation = CLIENT_REQUEST_GET_STDERR;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'h':
                printf("%s", usage_info);
                return 0;
            case '?':
                fprintf(stderr, "%s", usage_info);
                goto error;
        }
    }

    // Concatenation du chemin vers les pipes si l'option -p n'est pas renseignée
    if(pipes_directory == NULL) {
        directory_basename = concat_strings("/tmp/",getenv("USER"), "/saturnd/pipes");
    }else{
        directory_basename = pipes_directory;
    }
    
    struct timing* time = init_timing(minutes_str, hours_str, daysofweek_str);

    if(write_request_cassini(operation,taskid, time,directory_basename ,argc, argv, optind) == 1){
        goto error;
    }
    if(show_reply_daemon(directory_basename, operation) == 1){
        goto error;
    }

    free(time);
    time = NULL;
    free(directory_basename);
    directory_basename = NULL;

    return EXIT_SUCCESS;

    error:
    if (errno != 0) perror("main");
    free(time);
    time = NULL;
    if(pipes_directory == NULL) {
        free(directory_basename);
        directory_basename = NULL;
    }
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                             FONCTION ECRITURE REQUETES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Fonction principale qui envoie les requêtes en fonction de l'opération choisie
*   @param operation 
*   @param taskid
*   @param time
*   @param path
*   @param argc
*   @param argv
*   @indice_arg
*/
int write_request_cassini (uint16_t operation, uint64_t taskid, struct timing* time, char* path, int argc, char** argv, int indice_arg){
    // little endian to big endian ( bit inversés)
    uint16_t operation_bis = htobe16(operation);
    uint64_t taskid_bis = htobe64(taskid);
    char request_buf[512];
    char* buf_cursor = request_buf;
    int buf_size = 0;

    char* request = (concat_strings(path,"/", "saturnd-request-pipe"));
    if(request == NULL){
        perror("ecriture du request");
        goto error;
    }
    int ecriture_requete = open(request, O_WRONLY);
    if(ecriture_requete == -1){
        perror("Ouverture ecriture requete Cassini");
        goto error;
    }
    memmove(request_buf,&operation_bis,2);
    buf_cursor = buf_cursor + 2;
    buf_size = buf_size + 2;

    if ((operation == CLIENT_REQUEST_GET_STDERR ) || (operation == CLIENT_REQUEST_GET_STDOUT) ||
        (operation ==  CLIENT_REQUEST_REMOVE_TASK) || (operation == CLIENT_REQUEST_GET_TIMES_AND_EXITCODES) ){

        memmove(buf_cursor,&taskid_bis,8);
        buf_size = buf_size + 8;
       
    }
    if (operation == CLIENT_REQUEST_CREATE_TASK){
        uint64_t minutes_bis = htobe64(time -> minutes);
        uint32_t hours_bis = htobe32(time -> hours);

        memmove(buf_cursor,&minutes_bis,sizeof(minutes_bis));
        buf_cursor = buf_cursor + sizeof(minutes_bis);
        buf_size = buf_size + sizeof(minutes_bis);

        memmove(buf_cursor,&hours_bis,sizeof(hours_bis));
        buf_cursor = buf_cursor + sizeof(hours_bis);
        buf_size = buf_size + sizeof(hours_bis);

        memmove(buf_cursor,&(time -> daysofweek),sizeof(time -> daysofweek));
        buf_cursor = buf_cursor + sizeof(time -> daysofweek);
        buf_size = buf_size + sizeof(time -> daysofweek);

        send_commandline(buf_cursor,&buf_size, argc, argv, indice_arg);
        
    }
    if((write(ecriture_requete, request_buf, buf_size)) == -1){
            perror("write request buffer");
            goto error;
        }


    close(ecriture_requete);
    free(request);
    return 0;

    error :
    close(ecriture_requete);
    free(request);
    request = NULL;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                          FONCTION LECTURE ET AFFICHAGE "SERVER_REPLY"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Fonction principale d'affichage permettant de lire dans le tube réponse du démon et nous afficher
 * sa réponse selon l'opération écrite dans le client cassini
 *
 * @param path :  "chemin vers le pipe"
 * @param operation : "server_request"
 * @return void
 */

int show_reply_daemon (char* path , uint16_t operation) {

    //on déclare les variables dans lesquelles on va stocker nos elements d'affichage selon l'opération
    uint16_t reptype;
    uint64_t taskid;
    uint16_t errcode;

    char* reply = (concat_strings(path,"/", "saturnd-reply-pipe"));

    int reponse = open(reply, O_RDONLY);

    if(reponse == -1){
        perror("Ouverture réponse démon");
        goto error;
    }

    // on lit le premier bloc dans le tube réponses "reptype" et on le stocke dans &reptype
    if(read(reponse, &reptype, sizeof(reptype)) == -1){
        perror("erreur lecture reptype du démon");
        goto error;
    }

    // on détermine ce qu'on doit afficher selon l'opération envoyée par cassini 
    switch (operation) {

                case CLIENT_REQUEST_LIST_TASKS :// 'LS' (LIST)
                //fonction auxilaire qui affiche la réponse de la requête LIST
                if(show_reply_LS(reponse) == 1){
                    goto error;
                }
                break;

                case CLIENT_REQUEST_CREATE_TASK : // 'CR' (CREATE)

                    if(read(reponse, &taskid, sizeof(taskid)) == -1){
                        perror("erreur lecture taskid de la tâche 'CR'");
                        goto error;
                    }
                     printf(" %u ", (unsigned int) be64toh(taskid));
                     break;

                case CLIENT_REQUEST_REMOVE_TASK : // 'RM' (REMOVE) a deux réponses possibles

                    //si le reptype est 'ER'
                    if (reptype == SERVER_REPLY_ERROR) {

                        if(read(reponse, &errcode, sizeof(errcode)) == -1){
                            perror("erreur lecture errcode de la tache 'RM' ");
                            goto error;
                        }}

                    // Sinon on n'affiche rien
                    break;

                case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES: // 'TX' (TIME EXIT CODE)

                        //fonction auxiliaire qui affiche la réponse du démon à la requête "TX"
                        if(show_reply_TX(reponse, reptype) == 1){
                            goto error;
                        }

                        break;

                case CLIENT_REQUEST_TERMINATE : // 'TM' pour terminate

                     // aucun affichage

                    break;

                // CLIENT_REQUEST_GET_STDOUT 'SO' stdout  ou  CLIENT_REQUEST_GET_STDERR  'SE' stderr
                case CLIENT_REQUEST_GET_STDOUT:
                    case CLIENT_REQUEST_GET_STDERR:
                       //fonction auxiliaire lit la réponse et l'affiche (les deux request ont le même affichage)
                       if(show_reply_SO_SE(reponse,reptype) == 1){
                           goto error;
                       }
                    break;

                default:
                    perror (" Pas d'opération valide sélectionnée");
                    goto error;
              }
    close(reponse);
    free(reply);
    return 0;

    error :
    close(reponse);
    free(reply);
    reply = NULL;
    return 1;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                           FONCTIONS AUXILIAIRES UTILES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**********************************************************************************************************************/
/**
 * fonction auxiliaire qui lit et affiche la réponse du démon en cas de request "TX"
 * @param reponse
 * @param reptype
 * @return void
 */
int show_reply_TX (int reponse, uint16_t reptype){

    //on déclare les variables dans lesquelles on va stocker nos elements d'affichage selon l'opération

    uint32_t nbruns;
    int64_t time;
    uint16_t exitcode;
    uint16_t errcode;
    char buffer[MAX_SIZE];
    struct tm *timeinfo;

    // si le demon répond par 'ER'
    if (reptype == htobe16(SERVER_REPLY_ERROR)) {

        if(read(reponse, &errcode, sizeof(errcode)) == -1){
           perror("erreur lecture errcode de la tache 'TX' ");
           return 1;
        }

        //Si errcode est égale à 'NF' ou 'NR' on termine le programme 
        if (errcode == htobe16(SERVER_REPLY_ERROR_NOT_FOUND) || errcode == htobe16(SERVER_REPLY_ERROR_NEVER_RUN)){
            return 1;
        }

    //Si le demon répond par 'OK'
    }else{

        if(read(reponse, &nbruns, sizeof(nbruns)) == -1){
            perror("erreur lecture nbruns de la tache 'TX' ");
            return 1;
        }

        // On affiche chaque time exit code
        nbruns = be32toh(nbruns);

        for (int i = 0; i < nbruns; i++) {


            if( read(reponse, &time, sizeof(time)) == -1){
                perror("erreur lecture time de la tache 'TX' ");
                return 1;
            }

            time = (unsigned) htobe64(time);

            //création de la structure pour le temps
            timeinfo = localtime(&time);

            if (timeinfo == NULL) {
                perror("échec allocation mémoire pour timeinfo dans show_reply_TX");
                return 1;
            }

            //écriture du temps au bon format dans un buffer
            strftime(buffer, MAX_SIZE, "%F %T ", timeinfo);

            printf("%s", buffer);


            if( read(reponse, &exitcode, sizeof(exitcode)) == -1){
                perror("erreur lecture exitcode de la tache 'TX' ");
                return 1;
            }

            printf("%u \n", (unsigned) exitcode);
        }

    }
    return 0;

}

/**********************************************************************************************************************/
/**
 * fonction auxiliaire qui lit la réponse du démon à la requête 'LS' et qui l'affiche
 * @param reponse
 * @return void
 */
int show_reply_LS ( int reponse){

    //on déclare les variables dans lesquelles on va stocker nos elements d'affichage selon l'opération
    uint64_t taskid;
    uint32_t nbtasks;
    uint32_t argc;
    uint32_t longueur_command;
    struct timing* timing;
    char buf[TIMING_TEXT_MIN_BUFFERSIZE] ;
    char *command ;


    if( read(reponse, &nbtasks, sizeof(nbtasks)) == -1){
        perror("erreur lecture nbtasks de la tache 'LS' ");
        goto error;
    }

    // On affiche chaque tache
    nbtasks = be32toh(nbtasks);

    for (int i = 0; i < nbtasks; i++) {

        if(read(reponse, &taskid, sizeof(taskid)) == -1){
            perror("erreur lecture taskid de la tache 'LS' ");
            goto error;
        }

        printf("%u:", (unsigned int) be64toh(taskid));

        //lecture de timing:  ( minutes, heures, jours)
        timing = malloc(sizeof (struct timing));
        if(timing==NULL){
            perror("échec malloc pour timing dans show_reply_SO_SE");
            goto error;
        }

        if(read(reponse, &(timing -> minutes), sizeof(timing -> minutes)) == -1){
            perror("erreur lecture minutes) de la tache 'LS' ");
            goto error;
        }

        if(read(reponse, &(timing -> hours), sizeof(timing -> hours)) == -1){
            perror("erreur lecture hours de la tache 'LS' ");
            goto error;
        }

        if(read(reponse, &(timing -> daysofweek), sizeof(timing -> daysofweek)) == -1){
            perror("erreur lecture daysofweek de la tache 'LS' ");
            goto error;
        }

        //stockage des minutes des heures et des jours dans une structure après leur conversion
        struct timing time = {htobe64(timing -> minutes), htobe32(timing -> hours), timing -> daysofweek};

        timing_string_from_timing(buf, &time);
        printf(" %s ",buf);

        // On affiche les arguments des taches
        if(read(reponse, &argc, sizeof(argc)) == -1){
            perror("erreur lecture argc  de commandlinede la tache 'LS' ");
            goto error;
        }

        argc = be32toh(argc);

        for (int j = 0; j < argc ; j++) {

            if(read(reponse, &longueur_command, sizeof(longueur_command)) == -1){
                perror("erreur lecture commandline de la taille de la tache 'LS' ");
                goto error;
            }

            longueur_command = be32toh(longueur_command);

            command = malloc(longueur_command +1);
            if(command==NULL){
                perror("échec malloc pour command dans show_reply_LS");
                free(command);
                goto error;
            }

            if(read(reponse, command, longueur_command) == -1){
                perror("erreur lecture command de la tache 'LS' ");
                free(command);
                goto error;
            }

            // on ajoute le caractère final et on affiche la chaine du commandline
            *(command + (longueur_command)) = '\0';
            printf("%s ", command);
            free(command);

        }
        printf("\n");
        free(timing);
    }
    return 0;

    error :
    free(timing);
    timing = NULL;
    return 1;
}
/**********************************************************************************************************************/
/**
 * fonction auxiliaire qui lit la réponse du démon à la requête 'SO' ou 'SE' et qui l'affiche
 * @param reponse
 * @param reptype
 * @return void
 */
int show_reply_SO_SE ( int reponse ,uint16_t reptype) {

    uint16_t errcode;
    uint32_t longueur_output;
    char *output;


    //si le démon répond par 'Ok'
    if (reptype == htobe16(SERVER_REPLY_OK)) {

        // On affiche la sortie standard
        if(read(reponse, &(longueur_output), sizeof(longueur_output)) == -1){
            perror("erreur lecture output de la tâche 'SO' ou 'SE' ");
            goto error;
        }

        longueur_output = be32toh(longueur_output);

        output = malloc(longueur_output + 1);
        if(output==NULL){
            perror("échec malloc pour command dans show_reply_SO_SE");
            free(output);
            goto error;
        }

        if(read(reponse, output, (longueur_output)) == -1){
            perror("erreur lecture output de la tâche 'SO' ou ''SE ");
            free(output);
            goto error;
        }
        *(output + (longueur_output)) = '\0';
        printf("%s", output);
        free(output);
        return 0;

        //sinon si démon répond 'ER'
    } else {

        // On affiche la sortie erreur
        if(read(reponse, &errcode, sizeof(errcode)) == -1){
            perror("erreur lecture errcode de la tâche 'SO' ou ''SE ");
            goto error;
        }

        // si le errcode est égale à 'NF' ou 'NR' on termine le programme 
        if (errcode == htobe16(SERVER_REPLY_ERROR_NOT_FOUND) || errcode == htobe16(SERVER_REPLY_ERROR_NEVER_RUN)) {
            goto error;
        }

        printf("%u", htobe16(errcode));
        return 0;
    }
    error :
    return 1;
}