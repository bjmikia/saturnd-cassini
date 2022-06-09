#include "../include/saturnd_options.h"
int list_tasks (int request,char *reply_path,char  *dirpath ) {

    /**
    * on ouvre dirpath (là ou on a les dossiers de toutes les tâches) on parcourt tt les dossiers
     * par ordre décroissant on affiche taskid ,timing ,commandline // reste à confirmer...
    */

    //déclaration des variables:
    DIR *dirp;
    struct dirent *entry;
    int res_timing;
    uint16_t reptype;
    uint32_t nbtasks;
    uint64_t taskid;
    uint32_t l;
    char* str;
    char* path_commandline;
    int nb_dir=0;
    int reply;
    int n1;
    int n2;
    //int length;
    //char buf[512];
     //char * buf_cursor = buf;
     //int buf_size = 0;
    uint32_t argc;
    //string* argv;
    char* taskid_path = NULL;


    // on ferme le descripteur request car on en a plus besoin :
    close(request);
    // on ouvre le pipe reply pour écrire dedans
    reply = open(reply_path,O_WRONLY);
    if(reply==-1){
        perror("error open pipe reply");
        goto error2;
    }
    // ouverture du répertoire
    dirp = opendir(dirpath);
    if (dirp == NULL) {
        perror("opendir");
        goto error2;
    }

    // On fait un premier parcours pour savoir le nombre de tasks :
    while ((entry=readdir(dirp)) != NULL) {
        // on ne vérifie pas si on a que des dossiers .. c'est supposés
        if ((strcmp(".", entry->d_name)) && (strcmp("..", entry->d_name))) {
            nb_dir++;
        }
    }
    reptype = htobe16(SERVER_REPLY_OK);
    nbtasks= htobe32((uint32_t) nb_dir);
    if (write(reply,&reptype,sizeof(reptype))==-1){
        perror("error write reptype");
        goto error2;
    }
    if (write(reply,&nbtasks,sizeof(nbtasks))==-1){
        perror("error write nbtasks");
        goto error2;
    }
    rewinddir(dirp);
    // une boucle pour parcourir toutes les tâches (hors . et ..)
    while ((entry = readdir(dirp)) != NULL) {

        //on ne regarde pas le contenu des répertoires ./ et ../

        if ((strcmp(".", entry->d_name)) && (strcmp("..", entry->d_name))) {

            // on met le taskid dans le tube reply apres l'avoir convertit en uint64

            sscanf((entry -> d_name), "%ld",&taskid);
            taskid =(uint64_t) (taskid);
            taskid = htobe64 (taskid);
            if (write(reply, &taskid, sizeof(taskid)) == -1) {
                perror("error write taskid");
                goto error2;
            }
            taskid_path = concat_strings(dirpath, "/", entry->d_name);
            path_commandline = concat_strings(taskid_path,"/", "commandline" );
            // on mets le timing de la tâche "entry->d_name"dans le tube
            res_timing = get_file_content(taskid_path, "timing" , reply);
            if (res_timing == -1) {
                perror("error putting file timing content in pipe");
                goto error;
            }
            // on mets la commandline de la tâche "entry->d_name"dans le buffer
            int cmd = open(path_commandline, O_RDONLY);
            if (cmd==-1){
                perror("error open commandline");
            }
            // récupérer le argc :
            if (read(cmd, &argc, sizeof (argc)) == -1) {
                perror("erreur lecture du argc ");
                goto error;
            }
            // on écrit le argc dans le pipe reply
            argc = htobe32(argc);
            if (write(reply, &argc, sizeof (argc)) == -1) {
                perror("write error argc");
                goto error;
            }

            for (int i=0;i<argc;i++){
                n1=read(cmd, &l, sizeof (l));
                if ( n1== -1) {
                    perror("erreur lecture du l su string ");
                    goto error;
                }
                n2=read(cmd, &str, l);
                if (n2 == -1) {
                    perror("erreur lecture du str ");
                    goto error;
                }
                l=htobe32(l);
                if (write(reply, &l, n1) == -1) {
                    perror("erreur lecture du l su string ");
                    goto error;
                }

                if (write(reply, &str, n2) == -1) {
                    perror("erreur lecture du str");
                    goto error;
                }
            }
            }




        }

    close(reply);
    closedir(dirp);
    if(taskid_path != NULL) {
        free(taskid_path);
    }
    return 0;

    error:
    free(taskid_path);
    free(path_commandline);
    close(reply);
    closedir(dirp);
    return 1;

    error2:
    close(reply);
    closedir(dirp);
    return 1;
}
