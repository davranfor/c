/*! 
 *  \brief     Chrono
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include "chrono.h"

int millisleep(long milliseconds)
{
    struct timespec msec;

    msec.tv_sec = milliseconds / 1000;
    msec.tv_nsec = (milliseconds % 1000) * 1000000;
    if (nanosleep(&msec, NULL) != 0)
    {
        perror("nanosleep");
        exit(EXIT_FAILURE);
    }
    return 1;
}

static long timespec_diff(const struct timespec *a, const struct timespec *b)
{
    struct timespec diff;

    diff.tv_sec = a->tv_sec - b->tv_sec;
    diff.tv_nsec = a->tv_nsec - b->tv_nsec;
    if (diff.tv_nsec < 0)
    {
        diff.tv_nsec += 1000000000;
        diff.tv_sec -= 1;
    }
    return diff.tv_sec * 1000 + diff.tv_nsec / 1000000;
}

struct chrono *chrono_new(void)
{
    return calloc(1, sizeof(struct chrono));
}

void chrono_init(struct chrono *chrono)
{
    chrono->start.tv_sec = 0;
    chrono->start.tv_nsec = 0;
    chrono->stop = 0;
}

static void chrono_run(struct chrono *chrono, int resume)
{
    struct timespec start;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    if (resume)
    {
        start.tv_sec -= chrono->stop / 1000;
        start.tv_nsec -= (chrono->stop % 1000) * 1000000;
        if (start.tv_nsec < 0)
        {
            start.tv_nsec += 1000000000;
            start.tv_sec -= 1;
        }
    }
    chrono->start = start;
    chrono->stop = 0;
}

void chrono_start(struct chrono *chrono)
{
    chrono_run(chrono, 0);
}

void chrono_resume(struct chrono *chrono)
{
    chrono_run(chrono, 1);
}

void chrono_stop(struct chrono *chrono) 
{
    chrono->stop = chrono_elapsed(chrono);
}

long chrono_elapsed(const struct chrono *chrono) 
{
    struct timespec now;

    if (chrono->start.tv_sec == 0)
    {
        return 0;
    }
    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return timespec_diff(&now, &chrono->start);
}

void chrono_destroy(struct chrono *chrono)
{
    free(chrono);
}

