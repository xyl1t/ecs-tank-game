#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void rotate_point(float* r_x, float* r_y, float x, float y, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    *r_x = x * c - y * s;
    *r_y = x * s + y * c;
}

bool entities_overlap(
    float x1, float y1, float r1, float x2, float y2, float r2) {
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) <=
           (r1 + r2) * (r1 + r2);
}

time_t getFileTimestamp(const char* path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtime;
}

float randf(void) { return (float)rand() / (float)(RAND_MAX); }
