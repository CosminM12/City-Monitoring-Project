#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LEN 64

void start_monitor() {
    pid_t hub_mon_pid = fork();

    if (hub_mon_pid == 0) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("Pipe failed!");
            exit(1);
        }

        pid_t monitor_pid = fork();
        if (monitor_pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            execl("./monitor_reports", "monitor_reports", NULL);
            perror("Failed to exec monitor_reports");
            exit(1);
        }

        close(pipefd[1]);

        char buff[256];
        int bytes_read;
        printf("[HUB] Monitor link established\n");

        while ((bytes_read = (int)read(pipefd[0], buff, sizeof(buff)-1)) > 0) {
            buff[bytes_read] = '\0';
            printf(">> [MONITOR]: %s", buff);

            if (strstr(buff, "ERROR") || strstr(buff, "EXIT")) {
                break;
            }
        }

        close(pipefd[0]);
        printf("[HUB] Monitor bg process has terminated\n");
        exit(0);
    }
}

void calculate_scores(char *args) {
    char *district = strtok(args, " \n");
    if (!district) {
        printf("Usage: calculate_scores <district1> [distrct2...]\n");
        return;
    }

    printf("\n=== WORKLOAD REPOORT ===\n");

    while (district != NULL) {
        int pipefd[2];
        pipe(pipefd);

        pid_t pid = fork();
        if (pid == 0) {
            //Scorer process
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            execl("./scorer", "scorer", district, NULL);
            printf("Failed to execute scorer for %s\n", district);
            exit(1);
        } else {
            //Read from scorer
            close(pipefd[1]);
            char buff[512];
            int bytes_read;

            //Read output from specific scorer
            while ((bytes_read = (int)read(pipefd[0], buff, sizeof(buff) - 1)) > 0) {
                buff[bytes_read] = '\0';
                printf("%s", buff);
            }

            close(pipefd[0]);
            waitpid(pid, NULL, 0); //Wait for scorer to finish
        }
        district = strtok(NULL, " \n");
    }

    printf("=============================\n\n");
}

int main() {
    char input[MAX_LEN];

    printf("=== City Infrastructure Hub ===\n");
    printf("Commands: start_monitor, calculate_scores <districts>, exit \n");

    while (1) {
        printf("Hub> ");

        if (!fgets(input, MAX_LEN, stdin)) break;

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        if (strncmp(input, "start_monitor", 13) == 0) {
            start_monitor();
        } else if (strncmp(input, "calculate_scores", 16) == 0) {
            calculate_scores(input + 17);
        } else if (strcmp(input, "exit") == 0) {
            printf("Exiting Hub...\n");
            break;
        } else {
            printf("Unknown command! Accepted commands: start_monitor, calculate_scores <districts>, exit\n");
        }
    }

    return 0;
}