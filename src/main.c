#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct Report {
    int id;
    char name[31];
    struct {
        float x;
        float y;
    } gps;
    char category[11];
    int severity;
    time_t timestamp;
    char desc[256];

} Report_t;

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


    return 0;
}
