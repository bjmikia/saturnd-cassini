#include "../include/saturnd_options.h"
int create_tasks_files(char * taskdir){
    int fd = 0;
    char *filename;

    filename = (concat_strings(taskdir,"/","timing" ));
    if(filename == NULL){
        perror("error file timing");
        return -1;
    }
    fd = creat(filename,0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }


    filename = (concat_strings(taskdir,"/","commandline" ));
    if(filename == NULL){
        perror("error file commandline");
        return -1;
    }

    fd = creat(filename, 0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }


    filename = (concat_strings(taskdir,"/","times" ));
    if(filename == NULL){
        perror("error file times");
        return -1;
    }

    fd = creat(filename,0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }


    filename = (concat_strings(taskdir,"/","exitcodes" ));
    if(filename == NULL){
        perror("error file exitcodes");
        return -1;
    }

    fd = creat(filename, 0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }


    filename = (concat_strings(taskdir,"/","stdout" ));
    if(filename == NULL){
        perror("error file stdout");
        return -1;
    }

    fd = creat(filename, 0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }


    filename = (concat_strings(taskdir,"/","stderr" ));
    if(filename == NULL){
        perror("error file stderr");
        return -1;
    }

    fd = creat(filename, 0700);
    if(fd == -1){
        perror("error creat");
        return -1;
    }

    return 0;
}



int execute(char *argv[],const char * dirtaskid, char * buf) {
    char *filename = NULL;
    int w;
    uint16_t exitcode;

    pid_t pid = fork();

    if(pid==0) {
        if(dirtaskid != NULL){
            /* redirection de la sortie standard*/
            filename = (concat_strings(dirtaskid,"/","stdout" ));
            if(filename == NULL){
                perror("error file stdout in execute");
                return -1;
            }
            int new_stdout = open(filename, O_WRONLY);
            if(new_stdout == -1){
                perror("open new_stdin");
                return -1;
            }
            dup2(new_stdout,STDOUT_FILENO);
            close(new_stdout);

            filename = (concat_strings(dirtaskid,"/","stderr" ));
            if(filename == NULL){
                perror("error file stderr in execute");
                return -1;
            }
            /*redirection de la sortie erreur*/
            int new_stderr = open(filename, O_WRONLY);
            if(new_stderr == -1){
                perror("open new_stderr");
                return -1;
            }
            dup2(new_stderr,STDERR_FILENO);
            close(new_stderr);
        }


        execvp(argv[0],argv);



    } else if(pid>0) {

        waitpid(pid,&w, WNOHANG);

        filename = (concat_strings(dirtaskid,"/","exitcodes" ));
        if(filename == NULL){
            perror("error file exitcodes in execute");
            return -1;
        }

        int exitfd= open(filename, O_WRONLY|O_APPEND);
        if(exitfd == -1){
            perror("open exitcodes");
            return -1;
        }

        if(WEXITSTATUS(w)==0){
            exitcode = 0;
            exitcode =  htobe16(exitcode);
        }else{
            exitcode = 0xFFFF;
            exitcode =  htobe16(exitcode);
        }


        int wrexit = write(exitfd,&exitcode,sizeof(exitcode));
        if(wrexit == -1){
            perror("open exitcodes");
            return -1;
        }
        close(exitfd);

    } else {

        return -1;
    }

    return 0;

}


int write_comm_time_infile(int requete,const char * dirtaskid){

    uint32_t argc;
    uint32_t longueur_command;
    char *command ;
    char *filename ;
    char buffer[512];
    char * buf_cursor = buffer;
    uint64_t minutes;
    uint32_t hours;
    uint8_t daysofweek;
    int nb2;

    filename = (concat_strings(dirtaskid,"/","timing" ));
    if(filename == NULL){
        perror("error file timing");
        return 1;
    }
    int time_file = open(filename,O_WRONLY);
    if(time_file == -1){
        return 1;
    }

    //lecture du timing via le tube et ecriture dans le fichier timing
    if(read(requete, &minutes, sizeof(minutes)) == -1){
        perror("erreur lecture minutes ");

    }
    htobe64(minutes);
    nb2 = write(time_file, &minutes,sizeof(minutes));
    if(nb2 == -1){
        return 1;
    }

    if(read(requete, &hours, sizeof( hours)) == -1){
        perror("erreur lecture hours ");

    }
    htobe32( hours);
    nb2 = write(time_file, &hours,sizeof( hours));
    if(nb2 == -1){
        return 1;
    }

    if(read(requete, & daysofweek, sizeof(daysofweek)) == -1){
        perror("erreur lecture daysofweek ");

    }

    nb2 = write(time_file, & daysofweek,sizeof(daysofweek));
    if(nb2 == -1){
        return 1;
    }

    close(time_file);

    filename = (concat_strings(dirtaskid,"/","commandline" ));
    if(filename == NULL){
        perror("error file commanline");
        return 1;
    }
    int comm_file = open(filename,O_WRONLY);
    if(comm_file == -1){
        return 1;
    }


    if(read(requete, &argc, sizeof(argc)) == -1){
        perror("erreur lecture argc  de commandline ");
    }
    argc = be32toh(argc);

    nb2 = write(comm_file, &argc,sizeof(argc));
    if(nb2 == -1){
        return 1;
    }

    char * argv[argc+1];


    for(int i  = 0; i < argc; i++){

        if(read(requete,&longueur_command, sizeof(longueur_command)) == -1){
            perror("erreur lecture de la taille ");

        }
        longueur_command = be32toh(longueur_command);
        nb2 = write(comm_file, &longueur_command, sizeof(longueur_command));
        if(nb2 == -1){
            return 1;
        }

        command = malloc(longueur_command +1);
        if(command==NULL){
            perror("échec malloc pour command ");
            free(command);
            return 1;

        }

        if(read(requete, command, longueur_command) == -1){
            perror("erreur lecture command ");
            free(command);
            return 1;
        }

        nb2 = write(comm_file,command,longueur_command);
        if(nb2 == -1){
            return 1;
        }
        command[-1] = '\0';

        memmove(buf_cursor,command,longueur_command +1) ;

        argv[i] = buf_cursor;
        buf_cursor = buf_cursor + longueur_command +1;

        free(command);


    }
    argv[argc]= NULL;
    close(requete);
    close(comm_file);

    filename = (concat_strings(dirtaskid,"/","times" ));
    if(filename == NULL){
        perror("error file times in execute");
        return 1;
    }

    int timesfd = open(filename, O_WRONLY|O_APPEND);
    if(timesfd == -1){
        perror("open times");
        return 1;
    }

        execute(argv,dirtaskid, buffer);


    return 1;

}



// Fonction Saturnd Create


int saturnd_create(const char * taskspath,const char* pathpipe,int requete, uint64_t bigTaskid){
    char  taskname[10];

    char* dirname;
    int reponse;
    uint64_t taskid= bigTaskid;

    int idtask = (int)taskid ;
    sprintf(taskname,"%d",idtask);
    dirname = (concat_strings(taskspath,"/", taskname));
    if(dirname == NULL){
        perror("error directory name");
        goto error;
    }
    int create_dir = mkdir(dirname,0700); //on cree le nouveau dossier
    if(create_dir == -1){
        perror("error creating repository");
        goto error;
    }


    int create_files =  create_tasks_files(dirname);
    if(create_files == -1){
        perror("error creating files");
        goto error;
    }


    // On ouvre le tube reply :
    reponse = open(pathpipe, O_WRONLY);
    if(reponse == -1){
        perror("error opening demon pipe");
        goto error;
    }

    uint16_t reptype = htobe16(SERVER_REPLY_OK);
    if ((write(reponse, &reptype,sizeof(reptype))) == -1) {
        perror("write reptype");
    }

    taskid = htobe64(taskid);
    if ((write(reponse, &taskid, sizeof(taskid))) == -1) {
        perror("write taskid");
    }


    int wrct =  write_comm_time_infile(requete,dirname);
    if(wrct == -1){
        perror("error writing in files comm timing");
        goto error;
    }

    close(reponse);
    free(dirname);
    return 0;

    error:
    close(requete);
    close(reponse);
    free(dirname);
    return 1;
}