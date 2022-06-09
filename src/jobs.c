#include "../include/saturnd_options.h"
int jobs_start(const char * taskspath) {
    char *minutes_str = "*";
    char *hours_str = "*";
    char *daysofweek_str = "*";
    struct timing *timing = init_timing(minutes_str, hours_str, daysofweek_str);
    DIR *dirp;
    struct dirent *entry;
    char *taskidpath = NULL;
    char *timingpath = NULL;
    uint64_t taskid;
    char *command;
    char *commandpath = NULL;
    char buffer[512];
    char *buf_cursor = buffer;
    uint64_t min;
    uint32_t hours;
    uint8_t days;
    uint32_t argc;
    uint32_t l;


    dirp = opendir(taskspath);
    if (dirp == NULL) {
        perror("opendir");
        goto error;
    }

    while ((entry = readdir(dirp)) != NULL) {

        //on ne regarde pas le contenu des répertoires ./ et ../

        if ((strcmp(".", entry->d_name)) && (strcmp("..", entry->d_name))) {

            // on met le taskid dans le tube reply apres l'avoir convertit en uint64

            sscanf((entry->d_name), "%ld", &taskid);
            taskid = (uint64_t) (taskid);
            taskid = htobe64 (taskid);

            taskidpath = concat_strings(taskspath, "/", entry->d_name);
            timingpath = concat_strings(taskidpath, "/", "timing");
            int timingfd = open(timingpath, O_RDONLY);
            if(timingfd == -1){
                perror (" error ouverture timing path in jobs");
                goto error;
            }
            if (read(timingfd, &min, sizeof(min)) == -1) {
                perror("error  read min in jobs");
                goto error;
            }

            if (read(timingfd, &hours, sizeof(hours)) == -1) {
                perror("error  read hours in jobs");
                goto error;
            }

            if (read(timingfd, &days, sizeof(days)) == -1) {
                perror("error  read day in jobs");
                goto error;
            }
            commandpath = concat_strings(taskidpath, "/", "commandline");
            int commandfd = open(commandpath, O_RDONLY);

            if(commandfd == -1){
                perror("error open commandfd in jobs");
            }

            if (read(commandfd, &argc, sizeof(argc)) == -1) {
                perror("erreur lecture du argc ");
                goto error;
            }

            argc = htobe32(argc);
            char *argv[argc + 1];

            for (int i = 0; i < argc; i++) {
                if (read(commandfd, &l, sizeof(l)) == -1) {
                    perror("erreur lecture du l su string ");
                    goto error;
                }
                l = htobe32(l);
                command = malloc(l + 1);
                if (command == NULL) {
                    perror("échec malloc pour command ");
                    goto error;
                }

                if (read(commandfd, command, l) == -1) {
                    perror("erreur lecture du str ");
                    goto error;
                }

                command[-1] = '\0';
                memmove(buf_cursor, command, l + 1);
                argv[i] = buf_cursor;
                buf_cursor = buf_cursor + l + 1;

                free(command);
            }
            argv[argc] = NULL;
            close(commandfd);
            close(timingfd);
            if ((min == (timing->minutes)) && (hours == (timing->hours)) && (days == (timing->daysofweek))) {
                int64_t t = time(NULL);
                t = htobe64(t);
                int wrtimes = write(timingfd, &t, sizeof(t));
                if (wrtimes == -1) {
                    perror("write times");
                    goto error;
                }
                close(wrtimes);
                execute(argv, taskidpath, buffer);
            }

        }
    }
        buf_cursor = buffer;
        free(commandpath);
        free(taskidpath);
        free(timingpath);
        free(buf_cursor);
        free(entry);
        free(timing);
        free(dirp);

        return 0;

        error:
        buf_cursor = buffer;
        if (commandpath != NULL) {
            free(commandpath);
        }
        if (taskidpath != NULL) {
            free(taskidpath);
        }
        if (timing != NULL) {
            free(timingpath);
        }

        free(buf_cursor);
        free(entry);
        free(timing);
        free(dirp);
        return 1;

}