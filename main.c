#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct report{
    int id;
    char inspectorName[20];
    float x;
    float y;
    char issueCategory[20];
    int severity;
    
}report;

int verifyDirectory(const char *path){
    struct stat stats;
    stat(path, &stats);
    if (S_ISDIR(stats.st_mode)){
        return 1;
    }
    return 0;
}

void add(char *district_name){
    if(!verifyDirectory(district_name)){
        mkdir(district_name, 'x');
        chdir(district_name);
        open();
        open();
        open();
    }

}

void list(){}

void view(){}

void remove_report(){}

void update_threshold(){}

void filter(){}

int main(int argc, char **argv){
    
}