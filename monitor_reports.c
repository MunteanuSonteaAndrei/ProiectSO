#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>

#define FILENAME ".monitor_pid"

void sigusr1_handler(int signal)
{       
    char buff[]="A new report has been added!\n";
    if (signal == SIGUSR1){
        write(STDOUT_FILENO, buff, sizeof(buff));
    }
  
}

void sigint_handler(int signal)
{       
    char buff[]="The program stopped(SIGINT)!\n";
    if (signal == SIGINT){
        write(STDOUT_FILENO, buff, sizeof(buff));
        unlink(FILENAME);
        exit(1);
    }
  
}

void set_signal_action1(void){
    
    struct sigaction act;
    
    bzero(&act, sizeof(act));
    
    act.sa_handler = &sigusr1_handler;
    
    sigaction(SIGUSR1, &act, NULL);
}

void set_signal_action2(void){
    
    struct sigaction act;
    
    bzero(&act, sizeof(act));
    
    act.sa_handler = &sigint_handler;
    
    sigaction(SIGINT, &act, NULL);
}

int main(void) {
    int fi;
    if( (fi = open(FILENAME, O_WRONLY, 0664)) == -1){
        fi = open(FILENAME, O_WRONLY | O_CREAT, 0664);
    }else{
        pid_t pid;
        read(fi, &pid, sizeof(pid_t));
        close(fi);

        char buff[20];
        sprintf(buff, "%d\n", pid);
        write(STDOUT_FILENO, buff, sizeof(buff));
        exit(1);
    }
    
    chmod(FILENAME, 0664);
    
    pid_t current_pid = getpid();
    
    write(fi, &current_pid, sizeof(pid_t));
    
    close(fi); 
    
    set_signal_action1();

    set_signal_action2();
    
    while(1){
        sleep(1);
        
    }

    return 0;
}