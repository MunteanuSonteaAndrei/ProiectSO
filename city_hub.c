#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 256

void executa_start_monitor() {
    pid_t hub_mon = fork();
    
    if (hub_mon < 0) {
        perror("Eroare la fork pentru hub_mon");
        return;
    }
    
    if (hub_mon == 0) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("Eroare la crearea pipe-ului");
            exit(1);
        }
        
        pid_t monitor_pid = fork();
        if (monitor_pid < 0) {
            perror("Eroare la fork pentru monitor");
            exit(1);
        }
        
        if (monitor_pid == 0) {
            close(fd[0]);
            
            dup2(fd[1], STDOUT_FILENO); 
            close(fd[1]);
            
            char *args[] = {"./monitor_reports", NULL};
            execvp(args[0], args);
            perror("Eroare la execvp pentru monitor_reports");
            exit(1);
        }
        close(fd[1]);

        FILE *pipe_stream = fdopen(fd[0], "r");
        if (pipe_stream == NULL) {
            perror("Eroare la fdopen");
            exit(1);
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe_stream) != NULL) {
            printf("[HUB_MON] %s", buffer);
            fflush(stdout);
        }
        
        fclose(pipe_stream);

        int status;
        waitpid(monitor_pid, &status, 0);
        
        printf("[HUB_MON] Programul monitor s-a terminat (procesul a ieșit).\n");
        fflush(stdout);
        
        exit(0); 
    }
}

int main() {
    char input[MAX_LINE];
    
    printf("Comenzi: start_monitor, exit\n\n");
    
    while (1) {
        printf("city_hub> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nInchidere fortata prin EOF.\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "start_monitor") == 0) {
            executa_start_monitor();
        } else if (strcmp(input, "exit") == 0) {
            printf("Inchidere city_hub.\n");
            break;
        } else if (strlen(input) > 0) {
            printf("Comanda necunoscuta: '%s'. Foloseste 'start_monitor' sau 'exit'.\n", input);
        }
    }
    
    return 0;
}