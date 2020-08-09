#ifndef TASK_H
#define TASK_H

enum
{
    STATE_START,
    STATE_DRAW,
    STATE_STOP,
    STATES
};

enum
{
    TASK_NONE,
    TASK_MENU,
    TASK_PLAY,
    TASK_EXIT,
    TASKS,
};

typedef int (callback_t)(int);

void game_menu(game_t *, callback_t *task[]);
void game_play(game_t *, callback_t *task[]);
void game_exit(game_t *, callback_t *task[]);

#endif /* TASK_H */

