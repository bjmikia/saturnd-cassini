#include "../include/saturnd_options.h"
int remove_task (int request,char *reply_path,char  *dirpath ){

    /**
     * ici on donne le dossier qui contient tous les dossiers des tâches
     * on cherche le dossier avec le nom taskid dans le dossier dirpath
     * et on le supprime (récursivement s'il n'est pas vide)
     *
     * => on considère par défaut que taskid ne contient que des fichiers
     * et que dirpath ne contient que des dossiers
     */
    //déclaration des variables

    DIR *taskDir;
    struct dirent *task;
    int reply;
    uint64_t taskid;
    uint16_t reptype;
    uint16_t errcode;
    errno=0;

    // on lit le taskid et on ferme le descripteur du tube request
    if(read(request, &taskid, sizeof(taskid)) == -1){
        perror("erreur lecture du taskid");
        close(request);
        goto error;
    }
    close(request);
    ///on le transfome en char*
    taskid= htobe64(taskid);
    taskid = (int) (taskid);

    char char_taskid[4];
    sprintf(char_taskid,"%ld",taskid);
    // ouverture du tube reply pour l'écriture de la réponse
    reply = open(reply_path,O_WRONLY);
    if(reply==-1){
        perror("error open pipe reply");
        goto error;
    }
    // on récupère le chemin vers le dossier tâche
    char * path = concat_strings(dirpath,"/",char_taskid);
    // on tente l'ouverture du dossier taskid :
    taskDir  = opendir(path);
    // si le répertoire n'exist pas AUCUNE TACHE N'EXISTE
    if(taskDir == NULL) {
        reptype = htobe16(SERVER_REPLY_ERROR);
        errcode = htobe16(SERVER_REPLY_ERROR_NOT_FOUND);
        if (write(reply,&reptype,2)==-1){
            perror("write reptype");
            goto error;
        }
        if (write(reply,&errcode,2)==-1){
            perror("write errcode");
            goto error;
        }
        closedir(taskDir);
        close(reply);
        return 0;
    }

    // si le dossier existe : on le parcourt et on le supprime récursivement
    while ((task = readdir(taskDir))) {
        //on skip les répertoires ./ et ../
        if (!strcmp(task->d_name, ".") || !strcmp(task->d_name, "..")) continue;
        if (remove(concat_strings(path,"/",task->d_name)) == -1) {
            perror("error removing block(file)");
            goto error;
        }
    }
    // mtn notre dossier est vide on le supprime
    rmdir(path) ;
    // on écrit la réponse du démon dans le buf
    reptype = htobe16(SERVER_REPLY_OK);
    if (write(reply,&reptype,2)==-1){
        perror("write reptype");
        goto error;
    }

    closedir(taskDir);
    close(reply);
    return 0;
    error:
    close(reply);
    closedir(taskDir);
    return 1;

}
