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

void buildPath(char *dest, const char *district, const char *filename) {
    snprintf(dest, MAX_PATH, "%s/%s", district, filename);
}

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
    if (stat(path, &st) == -1) {
        char dirPath[MAX_PATH];
        strncpy(dirPath, path, MAX_PATH - 1);
        dirPath[MAX_PATH - 1] = '\0';
        char *slash = strrchr(dirPath, '/');
        if (slash) *slash = '\0';
        else strcpy(dirPath, ".");
        
        if (stat(dirPath, &st) == -1) return 0;
    }
    
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

void logOperation(const char *district, const char *action) {
    char path[MAX_PATH];
    buildPath(path, district, "logged_district");

    if (!hasPermission(path, S_IWUSR, 0)) return; 

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        chmod(path, 0644); 
        char entry[256];
        snprintf(entry, sizeof(entry), "[%ld] Role: %s | User: %s | Act: %s\n", 
                 time(NULL), currentRole, currentUser, action);
        write(fd, entry, strlen(entry));
        close(fd);
    }
}

int parseCondition(const char *input, char *field, char *op, char *value) {
    char temp[256];
    strncpy(temp, input, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *token = strtok(temp, ":");
    if (!token) return 0;
    strncpy(field, token, 31); field[31] = '\0';

    token = strtok(NULL, ":");
    if (!token) return 0;
    strncpy(op, token, 3); op[3] = '\0';

    token = strtok(NULL, "");
    if (!token) return 0;
    strncpy(value, token, 63); value[63] = '\0';

    return 1;
}

int matchCondition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, "<") == 0) return r->severity < val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
        if (strcmp(op, ">") == 0) return r->severity > val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
    } else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
    } else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector, value) != 0;
    } else if (strcmp(field, "timestamp") == 0) {
        time_t val = (time_t)atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, ">") == 0) return r->timestamp > val;
        if (strcmp(op, "<") == 0) return r->timestamp < val;
    }
    return 0;
}

void initDistrict(const char *district) {
    struct stat st;
    if (stat(district, &st) == -1) {
        mkdir(district, 0750);
        chmod(district, 0750);
    }
    
    char symlinkName[MAX_PATH], targetPath[MAX_PATH];
    snprintf(symlinkName, MAX_PATH, "active_reports-%s", district);
    buildPath(targetPath, district, "reports.dat");
    
    unlink(symlinkName); 
    symlink(targetPath, symlinkName);
}

int getNextId(const char *district) {
    char path[MAX_PATH];
    buildPath(path, district, "next_id.txt");
    int id = 1;
    
    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        char buf[32];
        ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            id = atoi(buf);
        }
        close(fd);
    }
    
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd != -1) {
        chmod(path, 0664);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d\n", id + 1);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    
    return id;
}

void add(const char *district) {
    char path[MAX_PATH];
    buildPath(path, district, "reports.dat");
    
    if (!hasPermission(path, S_IWUSR, S_IWGRP)) return;

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd == -1) return;
    chmod(path, 0664);

    Report r;
    r.id = getNextId(district);
    printf("ID alocat automat: %d\n", r.id);
    
    strcpy(r.inspector, currentUser);
    printf("X: "); scanf("%f", &r.lat);
    printf("Y: "); scanf("%f", &r.lon);
    printf("Categorie (road/lighting/flooding): "); scanf("%31s", r.category);
    printf("Severitate (1-3): "); scanf("%d", &r.severity);
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    printf("Descriere scurta: "); fgets(r.description, 128, stdin);
    r.description[strcspn(r.description, "\n")] = 0;

    r.timestamp = time(NULL);
    write(fd, &r, sizeof(Report));
    close(fd);
    
    logOperation(district, "ADD");
    printf("Raport adaugat!\n");
}

void list(const char *district) {
    char path[MAX_PATH], symPath[MAX_PATH];
    buildPath(path, district, "reports.dat");
    snprintf(symPath, MAX_PATH, "active_reports-%s", district);

    struct stat symSt, targetSt;
    if (lstat(symPath, &symSt) == 0 && S_ISLNK(symSt.st_mode)) {
        if (stat(symPath, &targetSt) == -1) {
            printf("Avertisment: Linkul %s atarna in gol!\n", symPath);
        }
    }

    if (!hasPermission(path, S_IRUSR, S_IRGRP)) return;

    struct stat st;
    if (stat(path, &st) != -1) {
        char perm[10];
        modeToString(st.st_mode, perm);
        printf("Fisier: %s | Permisiuni: %s | Dimensiune: %ld bytes\n", path, perm, st.st_size);
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    Report r;
    printf("\nID\tInspector\tCategorie\tSeveritate\n");
    printf("--------------------------------------------------\n");
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("%d\t%s\t\t%s\t\t%d\n", r.id, r.inspector, r.category, r.severity);
    }
    close(fd);
    logOperation(district, "LIST");
}

void view(const char *district, int targetId) {
    char path[MAX_PATH];
    buildPath(path, district, "reports.dat");

    if (!hasPermission(path, S_IRUSR, S_IRGRP)) return;

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Eroare la deschiderea fisierului.\n");
        return;
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == targetId) {
            printf("\n--- Detalii Raport #%d ---\n", r.id);
            printf("Inspector: %s\n", r.inspector);
            printf("Coordonate GPS: (%.4f, %.4f)\n", r.lat, r.lon);
            printf("Categorie: %s\n", r.category);
            printf("Severitate: %d\n", r.severity);
            printf("Data inregistrarii: %s", ctime(&r.timestamp));
            printf("Descriere: %s\n", r.description);
            found = 1;
            break;
        }
    }
    close(fd);

    if (!found) {
        printf("Raportul cu ID %d nu a fost gasit in %s.\n", targetId, district);
    } else {
        logOperation(district, "VIEW");
    }
}

void remove_report(const char *district, int targetId) {
    if (strcmp(currentRole, "manager") != 0) {
        printf("Doar managerul poate sterge rapoarte!\n"); return;
    }

    char path[MAX_PATH];
    buildPath(path, district, "reports.dat");
    if (!hasPermission(path, S_IWUSR, 0)) return;

    int fd = open(path, O_RDWR);
    if (fd == -1) return;

    Report r;
    off_t targetPos = -1;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == targetId) {
            targetPos = lseek(fd, 0, SEEK_CUR) - sizeof(Report);
            break;
        }
    }

    if (targetPos == -1) {
        printf("Raportul %d nu exista.\n", targetId);
        close(fd); return;
    }

    struct stat st;
    fstat(fd, &st);
    off_t readPos = targetPos + sizeof(Report);
    off_t writePos = targetPos;

    while (readPos < st.st_size) {
        lseek(fd, readPos, SEEK_SET); read(fd, &r, sizeof(Report));
        lseek(fd, writePos, SEEK_SET); write(fd, &r, sizeof(Report));
        readPos += sizeof(Report);
        writePos += sizeof(Report);
    }

    ftruncate(fd, st.st_size - sizeof(Report));
    close(fd);
    
    logOperation(district, "REMOVE");
    printf("Raport sters cu succes.\n");
}

void updateThreshold(const char *district, int val) {
    if (strcmp(currentRole, "manager") != 0) {
        printf("Eroare: Doar managerii pot actualiza pragul.\n");
        return;
    }

    char path[MAX_PATH];
    buildPath(path, district, "district.cfg");

    struct stat st;
    if (stat(path, &st) == 0) {
        if ((st.st_mode & 0777) != 0640) {
            printf("Refuzat: Permisiunile fisierului cfg au fost alterate (nu sunt 640)!\n");
            return;
        }
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    if (fd == -1) return;
    chmod(path, 0640); 

    char buf[64];
    snprintf(buf, sizeof(buf), "THRESHOLD=%d\n", val);
    write(fd, buf, strlen(buf));
    close(fd);
    
    logOperation(district, "UPDATE_THRESHOLD");
    printf("Threshold actualizat la %d.\n", val);
}

void filter(const char *district, int condCount, char *conditions[]) {
    char path[MAX_PATH];
    buildPath(path, district, "reports.dat");
    
    if (!hasPermission(path, S_IRUSR, S_IRGRP)) return;

    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    Report r;
    printf("\nRapoarte filtrate:\nID\tInspector\tSeveritate\tCategorie\n");

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int matchAll = 1;
        
        for (int i = 0; i < condCount; i++) {
            char field[32], op[4], value[64];
            
            if (parseCondition(conditions[i], field, op, value)) {
                if (!matchCondition(&r, field, op, value)) {
                    matchAll = 0;
                    break; 
                }
            } else {
                printf("Eroare la parsarea conditiei: %s\n", conditions[i]);
            }
        }
        
        if (matchAll) {
             printf("%d\t%s\t\t%d\t\t%s\n", r.id, r.inspector, r.severity, r.category);
        }
    }
    close(fd);
    logOperation(district, "FILTER");
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        printf("Numar incorect de argumente!\n");
        return 1;
    }

    if (strchr(argv[6], '/') != NULL || strchr(argv[6], '.') != NULL) {
        printf("Nume de district invalid! Fara caractere speciale.\n");
        return 1;
    }

    strncpy(currentRole, argv[2], STR_LEN - 1);
    currentRole[STR_LEN - 1] = '\0';
    
    strncpy(currentUser, argv[4], STR_LEN - 1);
    currentUser[STR_LEN - 1] = '\0';
    
    char *cmd = argv[5];
    char *district = argv[6];

    initDistrict(district);

    if (strcmp(cmd, "--add") == 0) {
        add(district);
    } 
    else if (strcmp(cmd, "--list") == 0) {
        list(district);
    } 
    else if (strcmp(cmd, "--view") == 0) {
        if (argc < 8) { printf("Lipseste ID-ul raportului.\n"); return 1; }
        view(district, atoi(argv[7]));
    }
    else if (strcmp(cmd, "--remove_report") == 0) {
        if (argc < 8) { printf("Lipseste ID-ul raportului.\n"); return 1; }
        remove_report(district, atoi(argv[7]));
    }
    else if (strcmp(cmd, "--update_threshold") == 0) {
        if (argc < 8) { printf("Lipseste valoarea noului prag.\n"); return 1; }
        updateThreshold(district, atoi(argv[7]));
    }
    else if (strcmp(cmd, "--filter") == 0) {
        if (argc < 8) { printf("Lipsesc conditiile.\n"); return 1; }
        filter(district, argc - 7, &argv[7]);
    }
    else {
        printf("Comanda necunoscuta: %s\n", cmd);
    }
    return 0;
}