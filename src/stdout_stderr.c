#include "../include/saturnd_options.h"
int get_file_content (  char * dirpath ,char * filename,int fd){ // mettre -1 à la place de count

    int file;
    char *buffer = malloc (sizeof(MAX_LEN));
    if (buffer==NULL){
        perror("error allocation de mémoire");
        goto error;
    }
    char* path_filename = concat_strings(dirpath,"/",filename);

    //on ouvre le fichier à lire en lecture seulement
    file = open(path_filename, O_RDONLY);
    if (file == -1){
        perror("error openfile");
        goto error;
    }

    // la boucle qui va lire bloc par bloc jusqu'à la fin du fichier
    while (1){

        int   nb_oc_lu = read (file,buffer,MAX_LEN);
        if (nb_oc_lu ==-1 ){
            perror("error reading from file");
            goto error;
        }
        if (nb_oc_lu==0){
            break;
        }

        int nb_oc_ecrit  = write(fd,buffer,nb_oc_lu);
        if (nb_oc_ecrit ==-1 ){
            perror("error while writing in pipe");
            goto error;
        }

    }
    close(file);
    return 0;
    error:
    close(file);
    return 1;
}
int write_stderr_stdout (int request,char *reply_path, char * dirpath ,char* filename) {

    int res;
    int reply;
    uint16_t reptype;
    uint16_t errcode;
    uint64_t taskid;
    DIR *taskDir;
    struct stat file_stat;
    struct stat exit_code_stat;

    // on lit le taskid et on ferme le descripteur du tube request

    if (read(request, &taskid, sizeof(taskid)) == -1) {
        perror("erreur lecture du taskid");
        goto error;
    }
    ///on le transfome en char*
    taskid = htobe64(taskid);
    taskid = (int) ((taskid));
    char char_taskid[4];
    sprintf(char_taskid, "%ld", taskid);
    close(request);

    // ouverture du tube reply pour l'écriture de la réponse
    reply = open(reply_path, O_WRONLY);
    if (reply == -1) {
        perror("error open pipe reply");
        goto error;
    }

    // on récupère le chemin vers le dossier tâche
    char *path = concat_strings(dirpath, "/", char_taskid);
    // on tente l'ouverture du dossier taskid :
    taskDir = opendir(path);
    //if (errno==2) printf("n'existe paa");
    if (taskDir == NULL) {
        reptype = htobe16(SERVER_REPLY_ERROR);
        errcode = htobe16(SERVER_REPLY_ERROR_NOT_FOUND);
        if (write(reply, &reptype, 2) == -1) {
            perror("write reptype");
            goto error;
        }
        if (write(reply, &errcode, 2) == -1) {
            perror("write errcode");
            goto error;
        }
        closedir(taskDir);
        close(reply);
        return 0;
    } else {
        // si le répertoire existe : on cherche à savoir si la tâche a été exécuté :

        char* path_exitcode = concat_strings(path,"/","exitcodes");
        // on remplie la structure filestat par les caractéristiques de file exitcode pour savoir la taille

        if (stat(path_exitcode, &exit_code_stat) == -1) {
            perror("stat of file");
            goto error;
        }
        // si la taille=0 = la fonction n'a jamais été exécutée:
        if (exit_code_stat.st_size == 0) {

            //on écrit dans le tube reply du démon
            reptype = htobe16(SERVER_REPLY_ERROR);
            errcode = htobe16(SERVER_REPLY_ERROR_NEVER_RUN);
            if (write(reply, &reptype, 2) == -1) {
                perror("write reptype");
                goto error;
            }
            if (write(reply, &errcode, 2) == -1) {
                perror("write errcode");
                goto error;

            }

        } else {
            // si le fichier n'est pas vide :

            //on écrit dans le tube reply OK du démon
            reptype = htobe16(SERVER_REPLY_OK);
            if (write(reply, &reptype, 2) == -1) {
                perror("write reptype");
                goto error;
            }
            // on ouvre le dossier stdout et on récupe le output grâce à notre fonction get_file_content
            // pareil ici pas besoin de prendre stdout en paramètre
            // on remplie la structure file_stat par les caractéristiques de file (stdout/stderr)
            if (stat((concat_strings(path,"/",filename)), &file_stat) == -1) {
                perror("stat of file(stdout/stderr)");
                goto error;
            }
            uint32_t l = file_stat.st_size;
            if (write(reply, &l, 4) == -1) {
                perror("write length of string");
                goto error;
            }
            res = get_file_content(path, filename, reply);
            if (res == -1) {
                perror("error writing file content in pipe");
                goto error;
            }
        }
    }


    closedir(taskDir);
    close(reply);
    return 0;

    error:

    close(request);
    close(reply);
    closedir(taskDir);
    return 1;
}















    ///////////////////////////////////////////////////////////////////

