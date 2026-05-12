#ifndef REPORT_H
#define REPORT_H

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


#endif