/*! 
 *  \brief     Timer
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <errno.h>
#include "timer.h"

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

/* timer implementation */

struct timer
{
    long milliseconds, stop;
    struct timespec end;
    int version;
    int (*callback)(void *);
    void *data;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

static void timer_lock(struct timer *timer)
{
    if (pthread_mutex_lock(&timer->mutex) != 0)
    {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
}

static void timer_unlock(struct timer *timer)
{
    if (pthread_mutex_unlock(&timer->mutex) != 0)
    {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

struct timer *timer_new(long milliseconds, int (*callback)(void *), void *data)
{
    struct timer *timer = calloc(1, sizeof *timer);

    if (timer != NULL)
    {
        if (pthread_mutex_init(&timer->mutex, NULL) != 0)
        {
            perror("pthread_mutex_init");
            exit(EXIT_FAILURE);
        }
        if (pthread_cond_init(&timer->cond, NULL) != 0)
        {
            perror("pthread_cond_init");
            exit(EXIT_FAILURE);
        }
        timer->milliseconds = milliseconds;
        timer->callback = callback;
        timer->data = data;
    }
    return timer;
}

static void timer_config(struct timer *timer)
{
    struct timespec end;

    if (clock_gettime(CLOCK_REALTIME, &end) == -1)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    long milliseconds = timer->milliseconds;

    if (timer->stop > 0)
    {
        milliseconds -= timer->stop;
        timer->stop = 0;
    }
    end.tv_sec += milliseconds / 1000;
    end.tv_nsec += (milliseconds % 1000) * 1000000;
    if (end.tv_nsec >= 1000000000)
    {
        end.tv_nsec -= 1000000000;
        end.tv_sec += 1;
    }
    timer->end = end;
}

static void *timer_handler(void *handler) 
{
    struct timer *timer = handler;
    int version = timer->version;
    int stop = 0;

    for (;;)
    {
        timer_lock(timer);
        if (timer->version == 0)
        {
            /* There is a stop signal */
            timer_unlock(timer);
            return NULL;
        }
        if (version == timer->version)
        {
            if (stop == 1)
            {
                /* There is a timeout */
                timer->version = 0;
                timer_unlock(timer);
                return NULL;
            }
        }
        else
        {
            /* There is a start/reset/continue signal, synchronize the version */
            version = timer->version;
        }
        stop = 1;
        timer_config(timer);

        int error = pthread_cond_timedwait(&timer->cond, &timer->mutex, &timer->end);

        /*
         * if error = ETIMEDOUT: There is a timeout - run the callback (if exists)
         * if error = 0        : There is a signal - stop or reset the timer
         * in other case       : Abort the program
         */
        if (error == ETIMEDOUT)
        {
            stop = 0;
        }
        else if (error != 0)
        {
            perror("pthread_cond_timedwait");
            exit(EXIT_FAILURE);
        }
        timer_unlock(timer);
        if (stop == 0)
        {
            if ((timer->callback == NULL) || (timer->callback(timer->data) == 0))
            {
                stop = 1;
            }
        }
    }
}

static void timer_run(struct timer *timer, int resume)
{
    int version;

    timer_lock(timer);
    if (resume == 0)
    {
        timer->stop = 0;
    }
    version = timer->version++;
    pthread_cond_signal(&timer->cond);
    timer_unlock(timer);
    /*
     * If the thread was active a signal to reset the timer is sended and
     * in this case there is no need to create a new thread 
     */
    if (version == 0)
    {
        if (pthread_create(&timer->thread, NULL, timer_handler, timer) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}

void timer_start(struct timer *timer)
{
    timer_run(timer, 0);
}

void timer_continue(struct timer *timer)
{
    timer_run(timer, 1);
}

/*
 * Elapsed time
 *
 * Returns:
 * 0 if timer is not running or stopped
 * 0 if timer is not yet configured (thread starting)
 * timer->milliseconds if timer is busy (in a callback)
 * The elapsed time in other case
 *
 * Note:
 * timer->end is not atomic, do not use this function without a mutex
 */
static long timer_elapsed(const struct timer *timer)
{
    struct timespec now;

    if (timer->version == 0)
    {
        return 0;
    }
    if (timer->end.tv_sec == 0)
    {
        return 0;
    }
    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    long diff = timespec_diff(&now, &timer->end);

    if (diff < 0)
    {
        return timer->milliseconds + diff;
    }
    return timer->milliseconds;
}

void timer_stop(struct timer *timer)
{
    timer_lock(timer);
    timer->stop = timer_elapsed(timer);
    timer->version = 0;
    pthread_cond_signal(&timer->cond);
    timer_unlock(timer);
    if (pthread_join(timer->thread, NULL) != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
}

void timer_wait(struct timer *timer)
{
    if (pthread_join(timer->thread, NULL) != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
}

void timer_destroy(struct timer *timer)
{
    if (pthread_cond_destroy(&timer->cond) != 0)
    {
        perror("pthread_cond_destroy");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_destroy(&timer->mutex) != 0)
    {
        perror("pthread_mutex_destroy");
        exit(EXIT_FAILURE);
    }
    free(timer);
}

/* chrono implementation */

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

void chrono_continue(struct chrono *chrono)
{
    chrono_run(chrono, 1);
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

void chrono_stop(struct chrono *chrono) 
{
    chrono->stop = chrono_elapsed(chrono);
}

void chrono_destroy(struct chrono *chrono)
{
    free(chrono);
}

