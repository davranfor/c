/*! 
 *  \brief     Timer
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef TIMER_H
#define TIMER_H

#include <time.h>

int millisleep(long);

struct timer *timer_new(long, int (*)(void *), void *);
void timer_start(struct timer *);
void timer_continue(struct timer *);
void timer_stop(struct timer *);
void timer_wait(struct timer *);
void timer_destroy(struct timer *);
#define timer_reset timer_start

struct chrono
{
    struct timespec start;
    long stop;
};
struct chrono *chrono_new(void);
void chrono_init(struct chrono *);
void chrono_start(struct chrono *);
void chrono_continue(struct chrono *);
long chrono_elapsed(const struct chrono *);
void chrono_stop(struct chrono *);
void chrono_destroy(struct chrono *);
#define chrono_reset chrono_start

#endif /* TIMER_H */

