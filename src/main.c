#include <stdio.h>
#include <stdlib.h>
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

void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    if(mode & S_IRUSR) str[0] = 'r';
    if(mode & S_IWUSR) str[1] = 'w';
    if(mode & S_IXUSR) str[2] = 'x';

    if(mode & S_IRGRP) str[3] = 'r';
    if(mode & S_IWGRP) str[4] = 'w';
    if(mode & S_IXGRP) str[5] = 'x';

    if(mode & S_IROTH) str[6] = 'r';
    if(mode & S_IWOTH) str[7] = 'w';
    if(mode & S_IXOTH) str[8] = 'x';
}

//=====INIT=====
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

void list(const char *district, const char *role, const char *user) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if(!check_access(path, role, 1, 0)) {
        printf("Error: Access denied to read %s\n", path);
        return;
    }

    struct stat st;
    if(stat(path, &st) == 0) {
        char perm_str[10];
        mode_to_string(st.st_mode, perm_str);
        printf("File: %s | Perms: %s | Size: %ld bytes | Last Mod: %s\n",
               path, perm_str, st.st_size, ctime(&st.st_mtime));
    }

    int fd = open(path, O_RDONLY);
    if(fd < 0) return;

    Report_t report;
    while(read(fd, &report, sizeof(Report_t)) == sizeof(Report_t)) {
        printf("ID: %d | Cat: %s | Sev: %d | Insp: %s\n", report.id, report.category, report.severity, report.inspector);
    }

    close(fd);
    add_log(district, role, user, "list");
}

void view(const char *district, const char *role, const char *user, int target_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if(!check_access(path, role, 1, 0)) {
        printf("Error: Access denied to read: %s\n", path);
        return;
    }

    int fd = open(path, O_RDONLY);
    if(fd < 0)  return;

    Report_t r;
    bool found = false;
    while(read(fd, &r, sizeof(Report_t)) == sizeof(Report_t)) {
        if(r.id == target_id) {
            printf("--- Report %d ---\n", r.id);
            printf("Inspector: %s\nLocation: (%.2f lat, %.2f lon)\n", r.inspector, r.gps.x, r.gps.y);
            printf("Category: %s\nSeverity: %d\nTime: %s\nDescription: %s\n", r.category, r.severity, ctime(&r.timestamp), r.desc);
            found = true;
            break;
        }
    }

    close(fd);
    if(!found) {
        printf("Report %d not found!\n", target_id);
    } else {
        add_log(district, role, user, "view");
    }
}

void remove_report(const char *district, const char *role, const char *user, int target_id) {
    if(strcmp(role, "manager") != 0) {
        printf("Error: Access denied! Only managers can remove reports.\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = open(path, O_RDWR);
    if(fd < 0) return;

    struct stat st;
    fstat(fd, &st);

    int record_count = st.st_size / sizeof(Report_t);

    Report_t r;
    int report_index = -1;
    for(int i=0;i<record_count;i++) {
        read(fd, &r, sizeof(Report_t));
        if(r.id == target_id) {
            report_index = i;
            break;
        }
    }

    if(report_index == -1) {
        printf("Report %d not found!\n", target_id);
        close(fd);
        return;
    }

    for(int i = report_index + 1; i < record_count; i++) {
        lseek(fd, i * sizeof(Report_t), SEEK_SET);
        read(fd, &r, sizeof(Report_t));

        lseek(fd, (i-1)*sizeof(Report_t), SEEK_SET);
        write(fd, &r, sizeof(Report_t));
    }

    ftruncate(fd, (record_count-1) * sizeof(Report_t));
    close(fd);

    printf("Report %d removed successfully.\n", target_id);
    add_log(district, role, user, "remove_report");
}

void update_threshold(const char *district, const char *role, const char *user, const char *value) {
    if(strcmp(role, "manager") != 0) {
        printf("Error: Access denied! Only managers can update the threshold.\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district);

    struct stat st;
    if(stat(path, &st) == 0) {
        if((st.st_mode & 0777) != 0640) {
            printf("Security Error: Permission on %s have been altered. Aborting..\n", path);
            return;
        }
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if(fd >= 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "THRESHOLD=%s\n", value);
        write(fd, buf, strlen(buf));
        close(fd);
        printf("Threshold updated successfully to %s\n", value);
        add_log(district, role, user, "update_threshold");
        return;
    }

    close(fd);
    printf("Error writing threshold\n");

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

    else if (strcmp(command, "list") == 0) {
        list(district, role, user);
    } else if (strcmp(command, "view") == 0) {
        if (cmd_arg1) view(district, role, user, atoi(cmd_arg1));
    } else if (strcmp(command, "remove_report") == 0) {
        if (cmd_arg1) remove_report(district, role, user, atoi(cmd_arg1));
    } else if (strcmp(command, "update_threshold") == 0) {
        if (cmd_arg1) cmd_update_threshold(district, role, user, cmd_arg1);
    } else if (strcmp(command, "filter") == 0) {
        cmd_filter(district, role, user, argc, argv, cmd_start_idx);
    } else {
        printf("Unknown command: %s\n", command);
    }

    return 0;
}
