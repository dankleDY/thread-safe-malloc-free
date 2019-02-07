#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// doubly linked list to manage allocated space
// metadata header indicating the availablity and size of a block
struct _info_box {
  size_t size;
  struct _info_box *prev;
  struct _info_box *next;
};
typedef struct _info_box info_box;

// BOX is the size of the header
#define BOX sizeof(struct _info_box)

void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

void *nolock_malloc(size_t size);
void *lock_malloc(size_t size);

void nolock_free(void *p);
void lock_free(void *p);

info_box *nolock_findBlock(size_t s);
info_box *lock_findBlock(size_t s);

info_box *lock_allocateSpace(size_t s);
info_box *nolock_allocateSpace(size_t s);

void nolock_dislink(info_box * p);
void lock_dislink(info_box * p);

void nolock_link(info_box * p);
void lock_link(info_box * p);
