#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define MAX_PATH 256
#define STR_LEN 32

char currentRole[STR_LEN];
char currentUser[STR_LEN];

typedef struct {
    int id;
    char inspector[STR_LEN];
    float lat;
    float lon;
    char category[STR_LEN];
    int severity;
    time_t timestamp;
    char description[128];
} Report;

void modeToString(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

int hasPermission(const char *path, mode_t reqOwner, mode_t reqGroup) {
    struct stat st;
    if (stat(path, &st) == -1) return 1;
    if (strcmp(currentRole, "manager") == 0) {
        if ((st.st_mode & reqOwner) != reqOwner) {
            printf("Eroare! Managerul nu are permisiuni pe %s\n", path);
            return 0;
        }
    } else if (strcmp(currentRole, "inspector") == 0) {
        if ((st.st_mode & reqGroup) != reqGroup) {
            printf("Eroare! Inspectorul nu are permisiuni pe %s\n", path);
            return 0;
        }
    }
    return 1;
}

void add(char *district_name){}

void list(){}

void view(){}

void remove_report(){}

void update_threshold(){}

void filter(){}

int main(int argc, char **argv){
    
}