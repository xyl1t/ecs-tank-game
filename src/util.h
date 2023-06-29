#ifndef GAME_UTIL
#define GAME_UTIL
#include <stdbool.h>
#include <time.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/stat.h>
#endif

// for colored printfs, see: https://stackoverflow.com/a/23657072
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

void rotate_point(float* r_x, float* r_y, float x, float y, float angle);

bool entities_overlap(
    float x1, float y1, float r1, float x2, float y2, float r2);

time_t getFileTimestamp(const char* path);

float randf(void);

#endif
