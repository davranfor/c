/*! 
 *  \brief     Chrono
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef CHRONO_H
#define CHRONO_H

#include <time.h>

int millisleep(long);

struct chrono
{
    struct timespec start;
    long stop;
};
struct chrono *chrono_new(void);
void chrono_init(struct chrono *);
void chrono_start(struct chrono *);
void chrono_resume(struct chrono *);
void chrono_stop(struct chrono *);
long chrono_elapsed(const struct chrono *);
void chrono_destroy(struct chrono *);
#define chrono_reset chrono_start

#endif /* CHRONO_H */

