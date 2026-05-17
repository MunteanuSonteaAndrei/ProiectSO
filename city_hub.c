#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>


int main(){
    int fd[2];
    pipe(fd);
    pid_t hub_mon = fork();
    if(hub_mon==0){
        close(fd[0]);
        dup2(fd[1], STDIN_FILENO);
        close(fd[1]);
        execvp("monitor_reports", "NULL");
        perror("Eroare!");
        return;
    }else{
        close(fd[1]);
        char b[128];
        while(1){
            read(fd[0], b, sizeof(b)-1);
            
        }
    }

}