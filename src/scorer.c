#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "report.h"

typedef struct InspectorScore {
    char name[MAX_STR];
    int total_severity;
} InspectorScore_t;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <district_name>\n", argv[0]);
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", argv[1]);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("[!]Distrct '%s' does not exist or has no reports.\n", argv[1]);
        return 1;
    }

    InspectorScore_t scores[100] = {0};
    int num_inspectors = 0;
    Report_t r;

    while (read(fd, &r, sizeof(Report_t)) == sizeof(Report_t)) {
        bool found = false;
        for (int i=0;i<num_inspectors;i++) {
            if (strcmp(scores[i].name, r.inspector) == 0) {
                scores[i].total_severity += r.severity;
                found = true;
                break;
            }
        }

        if (!found && num_inspectors < 100) {
            strncpy(scores[num_inspectors].name, r.inspector, MAX_STR);
            scores[num_inspectors].total_severity = r.severity;
            num_inspectors++;
        }
    }

    close(fd);

    printf("---Workload scores for %s---\n", argv[1]);
    for (int i=0;i<num_inspectors;i++) {
        printf("Inspector: %s | Total Workload Severity: %d\n", scores[i].name, scores[i].total_severity);
    }

    return 0;
}