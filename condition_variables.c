#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct thread_mutex {
  uint locked;
} lock;

struct q {
   struct thread_cond cv;
   struct thread_mutex m;
 
   void *ptr;
};

void
initlock(struct thread_mutex *lk, char *name)
{
  lk->locked = 0;
}

void
acquire(struct thread_mutex *lk)
{
  // The xchg is atomic.
  while(xchg(&lk->locked, 1) != 0){
   sleep(1);
  }
}

// Release the lock.
void
release(struct thread_mutex *lk)
{
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

volatile unsigned int delay (unsigned int d) {
   unsigned int i; 
   for (i = 0; i < d; i++) {
       __asm volatile( "nop" ::: );
   }

   return i;   
}

void
thread_mutex_init(struct thread_mutex *lk)
{
  char* name = "lock";
  initlock(lk, name);
}

void 
thread_mutex_lock(struct thread_mutex *lk)
{
  acquire(lk);
}

void
thread_mutex_unlock(struct thread_mutex *lk)
{
  release(lk);
}

// Initialize
thread_cond_init(&q->cv);
thread_mutex_init(&q->m);

// Thread 1 (sender)
void*
send(struct q *q, void *p)
{
   thread_mutex_lock(&q->m);
   while(q->ptr != 0)
      ;
   q->ptr = p;
   thread_cond_signal(&q->cv);
   thread_mutex_unlock(&q->m);
}

// Thread 2 (receiver)

void*
recv(struct q *q)
{
  void *p;

  thread_mutex_lock(&q->m);

  while((p = q->ptr) == 0)
    pthread_cond_wait(&q->cv, &q->m);
  q->ptr = 0;

  thread_mutex_unlock(&q->m);
  return p;
}
