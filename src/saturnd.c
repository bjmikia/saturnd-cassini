#include "saturnd.h"
#include <endian.h>

int create_pipes_and_directories (char* path_user, char* path_saturnd, char* path_pipes, char* task_directory, char* request,
                                  char* reply){
    struct stat file_stat;
    if (stat(path_user,&file_stat) < 0){
        if (mkdir(path_user, 0700) == -1){
            perror("erreur creation repertoire user ");
            return 1;
        }
        if (mkdir(path_saturnd, 0700) == -1){
            perror("erreur creation repertoire satrund ");
            return 1;
        }
        if (mkdir(path_pipes, 0700) == -1){
            perror("erreur creation repertoire pipes ");
            return 1;
        }
        if (mkdir(task_directory, 0700) == -1){
            perror("erreur creation repertoire task ");
            return 1;
        }
        if(mkfifo(request, 0600)){
            perror("erreur creation request pipe");
            return 1;
        }
        if(mkfifo(reply, 0600)){
            perror("erreur creation request pipe");
            return 1;
        }
    }
    return 0;
}
int read_request_write_reply () {
    DIR *dirp;
    struct dirent *entry;
    char *path_user = concat_strings("/tmp", "/", getenv("USER"));
    char *path_saturnd = concat_strings(path_user, "/", "saturnd");
    char *path_pipes = concat_strings(path_saturnd, "/", "pipes");
    char *task_directory = concat_strings(path_saturnd, "/", "taches");
    char *request = concat_strings(path_pipes, "/", "saturnd-request-pipe");
    char *reply = concat_strings(path_pipes, "/", "saturnd-reply-pipe");

    if (create_pipes_and_directories(path_user, path_saturnd, path_pipes, task_directory, request, reply) == 1) {
        goto error;
    }

    uint16_t operation;
    while(1) {
        uint64_t biggestTaskid = 0;
        int lecture_requete = open(request, O_RDONLY);
        if (lecture_requete == -1) {
            perror("Ouverture lecture requete");
            goto error;
        }
        // LECTURE REQUETE CASSINI
            if (read(lecture_requete, &operation, sizeof(operation)) == -1) {
                perror("erreur lecture de l'operation");
                close(lecture_requete);
                goto error;
            }
            uint16_t op = htobe16 (operation);
            switch (op) {
                case CLIENT_REQUEST_LIST_TASKS :// 'LS' (LIST)
                    if (list_tasks(lecture_requete, reply, task_directory) == 1) {
                        perror("list task error");
                        goto error;
                    }
                    break;

                case CLIENT_REQUEST_CREATE_TASK : // 'CR' (CREATE)
                    //  parcours pour savoir le plus grand taskid pour create
                    dirp = opendir(task_directory);
                    if (dirp == NULL) {
                        perror("opendir");
                        goto error;
                    }
                    // On fait un premier parcours pour savoir le nombre de tasks :
                    while ((entry=readdir(dirp)) != NULL) {
                        if ((strcmp(".", entry->d_name)) && (strcmp("..", entry->d_name))) {
                            int tmp;
                            sscanf((entry -> d_name), "%d",&tmp);
                            uint64_t tmp2 = (uint64_t) tmp;
                            if(biggestTaskid < tmp2){
                                biggestTaskid = tmp2;
                            }
                        }
                    }
                    biggestTaskid = biggestTaskid + 1;
                    if (saturnd_create(task_directory, reply, lecture_requete, biggestTaskid) == 1) {
                        perror("create task error");
                        goto error;
                    }
                    break;

                case CLIENT_REQUEST_REMOVE_TASK : // 'RM' (REMOVE)

                    if (remove_task(lecture_requete, reply, task_directory) == 1) {
                        perror("remove error");
                        goto error;
                    }
                    break;

                case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES: // 'TX' (TIME EXIT CODE)
                    if (write_exitcode(task_directory, reply, lecture_requete) == 1) {
                        perror("write times_exitcode error");
                        goto error;
                    }
                    break;

                case CLIENT_REQUEST_TERMINATE : // 'TM' ( TERMINATE)
                    terminate(reply, lecture_requete);
                    free(path_saturnd);
                    free(path_user);
                    free(path_pipes);
                    free(request);
                    free(reply);
                    free(task_directory);
                    exit(0);

                case CLIENT_REQUEST_GET_STDOUT: //'SO' (STDOUT)
                    if (write_stderr_stdout(lecture_requete, reply, task_directory, "stdout") == 1) {
                        perror("stdout error");
                        goto error;
                    }
                    break;

                case CLIENT_REQUEST_GET_STDERR: // 'SE' (STDERR)
                    if (write_stderr_stdout(lecture_requete, reply, task_directory, "stderr") == 1) {
                        perror("stdout error");
                        goto error;
                    }
                    break;
                default :
                    goto error;
            }
    }

    error:
    free(path_user);
    free(path_saturnd);
    free(path_pipes);
    free(request);
    free(reply);
    free(task_directory);
    exit(1);
}

void init_deamon(){
    pid_t pid;
    pid_t sid;
    pid_t pid2;
    pid = fork();
    switch (pid){
        case -1:
            exit(EXIT_FAILURE);
            break;

        case 0 :
            pid2 = fork();
            switch (pid2){
                case -1:
                    perror("Erreur fork2\n");
                    exit(EXIT_FAILURE);
                    break;

                case 0 :
                    umask(0);
                    read_request_write_reply();
                    break;

                default :
                    exit(EXIT_SUCCESS);
                    break;
            }
        default :
            exit(EXIT_SUCCESS);
            break;
    }
    sid = setsid();
    if (sid == -1 ){
        perror("sid failed");
        exit(EXIT_FAILURE);
    }
}

int main (void){
    init_deamon();
    return 0;
}