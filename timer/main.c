#include <stdio.h>
#include <stdatomic.h>
#include "timer.h"

struct counter
{
    _Atomic int value;
};

static int wakeup(void *data)
{
    struct counter *counter = data;

    printf("timer: %d\n", counter->value);
    if (counter->value++ == 50)
    {
        counter->value = -1;
        return 0;
    }
    return 1;
}

static void test_timer(void)
{
    struct counter counter = {0};
    struct timer *timer;

    puts("Testing timer");
    timer = timer_new(
        100,     // milliseconds
        wakeup,  // A callback function
        &counter // A pointer to the data
    );
    // Once set, you can change the timer duration casting to long:
    // *(long *)timer = 1000;
    timer_start(timer);
    while (counter.value <= 25)
    {
        millisleep(50);
    }
    timer_stop(timer);
    puts("Wait a second ...");
    millisleep(1000);
    timer_continue(timer);
    timer_wait(timer);
    timer_destroy(timer);
}

static void test_chrono(void)
{
    struct chrono chrono;

    puts("Testing chrono");
    chrono_start(&chrono);
    millisleep(1000);
    chrono_stop(&chrono);
    millisleep(100);
    chrono_continue(&chrono);
    millisleep(234);
    printf("chrono: %ld\n", chrono_elapsed(&chrono));
}

int main(void)
{
    test_timer();
    test_chrono();
    return 0;
}

