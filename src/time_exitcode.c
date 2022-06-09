#include "../include/saturnd_options.h"
int write_exitcode(const char * path_fichier, const char * path_reply, int requete) {
    uint64_t taskid;
    if (read(requete, &taskid, sizeof(taskid)) == -1) {
        perror("erreur lecture du taskid");
        close(requete);
        return 1;
    }
    taskid = htobe64(taskid);
    taskid = (int) taskid;
    char char_taskid[4];
    sprintf(char_taskid, "%ld", taskid);
    close(requete);

    char * path_task = concat_strings(path_fichier, "/", char_taskid);
    char * path_times = concat_strings(path_task, "/", "times");
    char * path_exit = concat_strings(path_task, "/", "exitcodes");
    char buf[512];
    char * buf_cursor = buf;
    int buf_size = 0;
    int file_exit;
    int file_times;
    struct stat filestat;
    int64_t time;
    uint16_t exitcode;
    uint32_t nbruns;
    uint16_t errcode = htobe16(SERVER_REPLY_ERROR_NOT_FOUND);
    uint16_t error;
    DIR * dir;

    int reply = open(path_reply, O_WRONLY);
    dir = opendir(path_task);
    if (dir == NULL) {
        error = htobe16(SERVER_REPLY_ERROR);
        memmove(buf_cursor, &error, sizeof(error));
        buf_cursor = buf_cursor + sizeof(error);
        buf_size = buf_size + sizeof(error);
        memmove(buf_cursor, &errcode, sizeof(errcode));
        if ((write(reply, buf, buf_size)) == -1) {
            perror("write error time exit code");
            goto error2;
        }
        close(reply);
        closedir(dir);
        free(path_task);
        return 0;
    } else {
        error = htobe16(SERVER_REPLY_OK);
        memmove(buf_cursor, &error, sizeof(error));
        buf_cursor = buf_cursor + sizeof(error);
        buf_size = buf_size + sizeof(error);

        file_exit = open(path_exit, O_RDONLY);
        if (file_exit == -1) {
            perror(" erreur ouverture exitcode ");
            goto error;
        }
        if (stat(path_exit, &filestat) == -1) {
            perror("stat of file");
            goto error;
        }
        nbruns = (uint32_t) ((filestat.st_size)/2);
        nbruns = htobe32 (nbruns);
        memmove(buf_cursor, &nbruns, sizeof(nbruns));
        buf_cursor = buf_cursor + sizeof(nbruns);
        buf_size = buf_size + sizeof(nbruns);
        printf("%d\n",nbruns);
        file_times = open(path_times, O_RDONLY);
        while (read(file_exit, &exitcode, sizeof(exitcode)) != 0) {

            if (read(file_times, &time, sizeof(time)) == -1) {
                perror("erreur lecture du time ");
                goto error;
            }
           /* if (read(file_exit, &exitcode, sizeof(exitcode)) == -1) {
                perror("erreur lecture du exitcode ");
                goto error;
            }*/
            if (buf_size >= 502) {
                if ((write(reply, buf, buf_size)) == -1) {
                    perror("write error time exit code");
                    goto error;
                }
            }
            time= htobe64(time);
            exitcode= htobe16(exitcode);
            memmove(buf_cursor, &time, sizeof(time));
            buf_cursor = buf_cursor + sizeof(time);
            buf_size = buf_size + sizeof(time);

            memmove(buf_cursor, &exitcode, sizeof(exitcode));
            buf_cursor = buf_cursor + sizeof(exitcode);
            buf_size = buf_size + sizeof(exitcode);
        }
        if ((write(reply, buf, buf_size)) == -1) {
            perror("write error time exit code");
            goto error;
        }

        closedir(dir);
        close(file_times);
        close(file_exit);
        close(reply);
        free(path_task);
        free(path_times);
        free(path_exit);
        return 0;
    }
    error:
    close(reply);
    closedir(dir);
    close(file_times);
    close(file_exit);
    free(path_task);
    free(path_times);
    free(path_exit);
    return 1;

    error2:
    close(reply);
    closedir(dir);
    free(path_task);
    free(path_times);
    free(path_exit);
    return 1;
}
