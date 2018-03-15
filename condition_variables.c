#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct thread_mutex {
  uint locked;
};

struct thread_cond {
  uint val;
};

struct q {
   struct thread_cond cv;
   struct thread_mutex m;
 
   void *ptr;
} q;

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


void 
thread_cond_init(struct thread_cond *cv)
{
  cv->val = 0;
}

void
thread_cond_wait(struct thread_cond* cv, struct thread_mutex* m){
  thread_mutex_unlock(m);
  if(cv->val > 0){
    //atomic exchange
    xchg(&cv->val, 0);
    thread_mutex_lock(m);
  } else {
    sleep(1);
    thread_mutex_lock(m);
  }
}

void
thread_cond_signal(struct thread_cond* cv){
  //atomic exchange
  xchg(&cv->val,1);
}


// Thread 1 (sender)
void
send(struct q *q, void *p)
{
   thread_mutex_lock(&q->m);
   while(q->ptr != 0){}
   q->ptr = p;
   printf(1, "Sending: %s\n", q->ptr);
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
    thread_cond_wait(&q->cv, &q->m);

  printf(1, "Got data: %s\n",q->ptr);
  q->ptr = 0;

  thread_mutex_unlock(&q->m);
  return p;
}

void 
produce(){
  char data[] = "My Data";
  send(&q,(void*)data);
  return;
}

void
receive(){
  recv(&q);
  thread_exit();
  return;
}

int
main(int argc, char *argv[]){
  // Initialize
  thread_cond_init(&q.cv);
  thread_mutex_init(&q.m);
  produce();
  receive();
  int dummy1 = 0, dummy2 = 0;
  void* s1 = malloc(4096);
  void* s2 = malloc(4096);
  thread_create(produce, (void*) &dummy1, s1);
  thread_create(receive, (void*) &dummy2, s2); 
  int r1 = thread_join();
  int r2 = thread_join();
  printf(1, "Thread finished: %d and %d\n",r1, r2);
  exit();
}
