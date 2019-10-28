/*! 
 *  \brief     Stack
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef STACK_H
#define STACK_H

typedef struct stack stack;

stack *stack_create(void);
void *stack_push(stack *, void *);
void *stack_pop(stack *);
void *stack_fetch(stack *, void **);
void *stack_head(stack *);
size_t stack_size(const stack *);
void stack_destroy(stack *, void (*)(void *));

#endif /* STACK_H */

