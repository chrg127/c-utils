#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

static inline size_t stack_grow_cap(size_t old_cap)
{
    return old_cap < 8 ? 8 : old_cap * 2;
}

typedef void *(*StackAllocator)(void *ptr, size_t old, size_t new);

static inline void *stack_default_allocator(void *ptr, size_t old, size_t new)
{
    (void) old;
    if (new == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, old, new);
}

#define STACK_DECLARE(Name, T, header)                              \
typedef struct Name {                                               \
    T *arr;                                                         \
    size_t top;                                                     \
    size_t cap;                                                     \
    StackAllocator allocator;                                       \
} Name;                                                             \
                                                                    \
void header##_init(Name *st, size_t max);                           \
void header##_init_allocator(Name *st, StackAllocator allocator);   \
void header##_push(Name *st, T v);                                  \
bool header##_pull(Name *st, T *value);                             \
void header##_free(Name *st);                                       \

#define STACK_DEFINE(Name, T, header)                               \
void header##_init(Name *st)                                        \
{                                                                   \
    st->data = NULL;                                                \
    st->top = 0;                                                    \
    st->cap = 0;                                                    \
    st->allocator = stack_default_allocator;                        \
}                                                                   \
                                                                    \
void header##_init_allocator(Name *st, StackAllocator allocator)    \
{                                                                   \
    st->data = NULL;                                                \
    st->top = 0;                                                    \
    st->cap = 0;                                                    \
    st->allocator = allocator;                                      \
}                                                                   \
                                                                    \
void header##_push(Name *st, T v)                                   \
{                                                                   \
    if (st->top == st->cap) {                                       \
        size_t old = st->cap;                                       \
        st->cap = old < 8 ? 8 : old * 2;                            \
        st->data = st->allocator(st->data, old, st->cap);           \
    }                                                               \
    st->data[st->top++] = v;                                        \
}                                                                   \
                                                                    \
bool header##_pull(Name *st, T *value)                              \
{                                                                   \
    if (st->top > 0) {                                              \
        *value = st->data[st->top--];                               \
        return true;                                                \
    }                                                               \
    return false;                                                   \
}                                                                   \
                                                                    \
void header##_free(Name *st)                                        \
{                                                                   \
    free(st->data);                                                 \
}                                                                   \

#define STACK_INIT() { .data = NULL, .top = 0, .cap = NULL, .allocator = vector_allocator }
#define STACK_INIT_ALLOCATOR(a) { .data = NULL, .top = 0, .cap = NULL, .allocator = a }

#endif
