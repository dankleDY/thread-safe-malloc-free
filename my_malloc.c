#include "my_malloc.h"

// head and tail pointer of the LL
__thread info_box *head = NULL;
__thread info_box *tail = NULL;

info_box *comm_head = NULL;
info_box *comm_tail = NULL;
pthread_mutex_t lock;

void *ts_malloc_lock(size_t size) {
  info_box *p = NULL;
  pthread_mutex_lock(&lock);
  p = lock_malloc(size);
  pthread_mutex_unlock(&lock);
  return p;
}

void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  lock_free(ptr);
  pthread_mutex_unlock(&lock);
};

// to find the best fit block which is free and has the closest size
void *lock_malloc(size_t size) {
  info_box *p = NULL;
  p = lock_findBlock(size);
  if (p) { // reusable block found
    lock_dislink(p);
  } else { // not found, allocate new space
    p = lock_allocateSpace(size);
  }
  return p + 1;
}

info_box *lock_findBlock(size_t s) {
  info_box *curr = comm_head;
  while (curr != NULL && curr->size < s) {
    curr = curr->next;
  }
  return curr;
}

info_box *lock_allocateSpace(size_t s) {
  info_box *p = NULL;
  p = (info_box *)sbrk(BOX + s);
  p->size = s;
  p->prev = NULL;
  p->next = NULL;
  return p;
}

void lock_dislink(info_box *p) {
  if (comm_head == comm_tail) { // dislink the only block
    comm_head = NULL;
    comm_tail = NULL;
    return;
  }
  if (!p->next) { // dislink tail
    if (!p->prev) {
      comm_head = NULL;
      comm_tail = NULL;
      return;
    }
    p->prev->next = NULL;
    comm_tail = p->prev;
    p->prev = NULL;
    return;
  }
  if (!p->prev) { // dislink head
    p->next->prev = NULL;
    comm_head = p->next;
    p->next = NULL;
    return;
  } //  dislink middle
  p->prev->next = p->next;
  p->next->prev = p->prev;
  p->prev = NULL;
  p->next = NULL;
}
// free memory. when two address-adjacent blocks are both free, merge them

void lock_free(void *p) {
  info_box *ptr = p;
  ptr--;
  lock_link(ptr);
}

void lock_link(info_box *p) {
  if (!comm_head) { // head is empty
    comm_head = p;
    comm_tail = p;
    return;
  }
  info_box *temp = comm_head;
  while (temp != NULL && temp->size < p->size) {
    temp = temp->next;
  }
  if (temp) {
    if (!temp->prev) { // insert at the head
      comm_head = p;
      temp->prev = p;
      p->next = temp;
    } else { // insert in the middle
      temp->prev->next = p;
      p->prev = temp->prev;
      temp->prev = p;
      p->next = temp;
    }
  } else { // insert at the tail
    comm_tail->next = p;
    p->prev = tail;
    comm_tail = p;
  }
}

void *ts_malloc_nolock(size_t size) {
  info_box *p = NULL;
  p = nolock_malloc(size);
  return p;
}

void ts_free_nolock(void *ptr) { nolock_free(ptr); }

void *nolock_malloc(size_t size) {
  info_box *p = NULL;
  p = nolock_findBlock(size);
  if (p) { // reusable block found
    nolock_dislink(p);
  } else { // not found, allocate new space
    p = nolock_allocateSpace(size);
  }
  return p + 1;
  // return p+1
  //       b/c p points to a info_box,
  //       what needed is the space this info_box connected to
  //       so with pointer arithmatic, we can move to the right address
}

info_box *nolock_findBlock(size_t s) {
  info_box *curr = head;
  // printf("      to find %ld \n",s);
  while (curr != NULL && curr->size < s) {
    curr = curr->next;
  }
  return curr;
}

info_box *nolock_allocateSpace(size_t s) {
  info_box *p = NULL;
  pthread_mutex_lock(&lock);
  p = sbrk(BOX + s);
  pthread_mutex_unlock(&lock);
  p->size = s;
  p->prev = NULL;
  p->next = NULL;
  // printf("      address %p\n",p);
  return p;
}

void nolock_free(void *p) {
  info_box *ptr = p;
  ptr--;
  nolock_link(ptr);
}

void nolock_link(info_box *p) {
  // printf("      freed %ld \n",p->size);
  if (!head) { // head is empty
    head = p;
    tail = p;
    return;
  }
  info_box *temp = head;
  while (temp != NULL && temp->size < p->size) {
    temp = temp->next;
  }
  if (temp) {
    if (!temp->prev) { // insert at the head
      head = p;
      temp->prev = p;
      p->next = temp;
    } else { // insert in the middle
      temp->prev->next = p;
      p->prev = temp->prev;
      temp->prev = p;
      p->next = temp;
    }
  } else { // insert at the tail
    tail->next = p;
    p->prev = tail;
    tail = p;
  }
  // printf("      linded %p \n",p);
}

void nolock_dislink(info_box *p) {
  if (head == tail) { // dislink the only block
    head = NULL;
    tail = NULL;
    return;
  }
  if (!p->next) { // dislink tail
    if (!p->prev) {
      comm_head = NULL;
      comm_tail = NULL;
      return;
    }
    p->prev->next = NULL;
    tail = p->prev;
    p->prev = NULL;
    return;
  }
  if (!p->prev) { // dislink head
    p->next->prev = NULL;
    head = p->next;
    p->next = NULL;
    return;
  } //  dislink middle
  p->prev->next = p->next;
  p->next->prev = p->prev;
  p->prev = NULL;
  p->next = NULL;
}
