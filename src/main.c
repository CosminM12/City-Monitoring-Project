#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>

#define MAX_STR 64
#define MAX_DESC 256

typedef struct Report {
    int id;
    char inspector[MAX_STR];
    struct {
        float x;
        float y;
    } gps;
    char category[11];
    int severity;
    time_t timestamp;
    char desc[MAX_DESC];

} Report_t;

//=====Utility=====
bool check_access(const char *filepath, const char *role, int read, int write) {
    if(!read && !write) {
        printf("Useless access function call\n");
        return 0;
    }

    struct stat st;
    if(stat(filepath, &st) < 0) {
        return false;
    }

    bool can_read=0, can_write=0;

    if(strcmp(role, "manager") == 0) {
        can_read = (st.st_mode & S_IRUSR) ? 1 : 0;
        can_write = (st.st_mode & S_IWUSR) ? 1 : 0;
    }
    else if(strcmp(role, "inspector") == 0) {
        can_read = (st.st_mode & S_IRGRP) ? 1 : 0;
        can_write = (st.st_mode & S_IWGRP) ? 1 : 0;
    }

    if(read && !can_read) return 0;
    if(write && !can_write) return 0;

    return 1;
}

void add_log(const char* district, const char *role, const char *user, const char *action) {
    char path[256];
    snprintf(path, sizeof(path), "%s/logged_district", district);

    if(!check_access(path, role, 0, 1)) {
        printf("Warning! User %s (role: %s) has no write permission to add log. Action not logged\n", user, role);
        return;
    }

    int fd = open(path, O_WRONLY || O_APPEND);
    if(fd >= 0) {
        char buffer[512];
        time_t now = time(NULL);
        snprintf((buffer, sizeof(buffer), "%ld\t%s\t%s\t%s\n"), now, user, role, action);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }
}


void init_district(const char *district) {
    struct stat st;

    //Check if district exits or create it (750)
    if(stat(district, &st) == -1) {
        mkdir(district);
        chmod(district, 0750);
    }

    char path[256];
    int fd;

    //Check for district.cfg (640)
    snprintf(path, sizeof(path), "%s/district.cfg", district);
    if(stat(path, &st) == -1) {
        fd = open(path, O_CREAT | O_WRONLY, 0640);
        if(fd >= 0) {
            write(fd, "THRESHOLD=2\n", 12);
            close(fd);
            chmod(path, 0640);
        }
    }

    //Check for logged_district (644)
    snprintf(path, sizeof(path), "%s/logged_district", district);
    if(stat(path, &st) == -1) {
        fd = open(path, O_CREAT | O_WRONLY, 0644);
        if(fd >= 0) {
            close(fd);
            chmod(path, 0644);
        }
    }

    //Check for reports.dat (664)
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    if(stat(path, &st) == -1) {
        fd = open(path, O_CREAT | O_WRONLY, 0664);
        if(fd >= 0) {
            close(fd);
            chmod(path, 0664);
        }
    }

    //Check for symbolic links
//    char symb_link[256];
//    snprintf(symb_link, sizeof(symb_link), "active-reports-%s", district);
//
//    struct stat lst;
//    if(lstat(symb_link, &lst) == 0) {
//        if(S_ISLNK(lst.st_mode)) {
//            if(stat(symb_link, &st) == -1) {
//                printf("Warning! Dangling symbolic link detected: %s. Recreating...\n", symb_link);
//                unlink(symb_link);
//                symlink(path, symb_link);
//            }
//        }
//        else {
//            symlink(path, symb_link);
//        }
//    }
}

//=====COMMANDS=====
void add(const char *district, const char *role, const char *user) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    //Check if user can write to file
    if(!check_access(path, role, 0, 1)) {
        printf("Error: Access denied to append to %s\n", path);
        return;
    }

    Report_t report;


    strncpy(report.inspector, user, MAX_STR);
    report.timestamp = time(NULL);

    printf("X: "); scanf("%f", &report.gps.x);
    printf("Y: "); scanf("%f", &report.gps.y);
    printf("Category (road/lighting/flooding/other): "); scanf("%s", report.category);
    printf("Severity level (1-3): "); scanf("%d", &report.severity);

    printf("Description: ");
    getchar();
    fgets(report.desc, MAX_DESC, stdin);
    report.desc[strcspn(report.desc, "\n")] = 0;

    int fd = open(path, O_RDWR);
    if(fd < 0) return;

    //Find ID
    struct stat st;
    fstat(fd, &st);
    if(st.st_size > 0) {
        report.id = (st.st_size / (int)sizeof(Report_t)) + 1;
    }

    //Write to file
    lseek(fd, 0, SEEK_END);
    write(fd, &report, sizeof(Report_t));
    close(fd);

    add_log(district, role, user, "add");
    printf("Report #%d added successfully\n", report.id);
}

int main(int argc, char* argv[]) {

    if(argc < 7) {
        printf("Usage: %s --role <role> --user <user> --<command> <district> [args]\n", argv[0]);
        return 1;
    }

    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    char *district = NULL;
    char *cmd_arg1 = NULL;
    int cmd_start_idx = 0;

    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        }
        else if(strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        }
        else if(strncmp(argv[i], "--", 2) == 0) {
            command = argv[i] + 2;
            if(i + 1 < argc) {
                district = argv[++i];
                if(i + 1 < argc) {
                    cmd_arg1 = argv[i+1];
                    cmd_start_idx = i + 1;
                }
            }
            break;
        }
    }

    if(!role || !user || !command || !district) {
        printf("Missing required arguments!\n");
        return 1;
    }



    init_district(district);


    if (strcmp(command, "add") == 0) {
        add(district, role, user);
    }

    return 0;
}
